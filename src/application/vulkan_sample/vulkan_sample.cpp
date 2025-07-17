
#include "vulkan_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../utils/blob.h"
#include "../../window/window.h"
#include "../../graphics/graphics.h"
#include <array>
#include <cstddef>
#include <cstring>
#include <thread>
#include <vector>
#include "../../graphics/camera/camera.h"
#include "../../input/input.h"
#include "../../../libs/imgui/imgui.h"

// todo: we don't want to have this as an explicit file like this I don't think.
#include "tiny_gltf_loader.h"
#include "GLFW/glfw3.h"
using namespace fw;
namespace fwvulkan
{
   extern GLFWwindow* g_window;
}
application* application::factory()
{
   return new vulkan_sample();
}
void vulkan_sample::init()
{
   m_name = "vulkan_sample";
   fw::commandline::parse();
   fw::log::topics::add("fw");
   
   fw::log::scope fw("fw");
   fw::log::topics::add("window");
   m_window = window::windowFactory();
   m_width = 1920; m_height = 1080;
   m_window->init(m_width, m_height, m_name);

   fw::log::topics::add("input");
   m_input = input::inputFactory();
   m_input->init();
   
   fw::log::topics::add("graphics");
   m_graphics = graphics::graphicsFactory();
   m_graphics->init();
   blob::init();
}

// todo: add pass dependencies.
// todo: move the swapchain mesh into framework.
// todo: move all imgui related init out of this sample and into framework.
// todo: lock the mouse center in freecam.
// todo: fix resource transition validation errors.
// todo: add pbr alpha support


void save_mesh(fw::Mesh* in_mesh)
{
   // explitily serialise each buffer, then load like below.
   // handle images etc next.
   blob::save("mesh/mesh.blob", blob::Buffer<fw::Mesh>({in_mesh, 1}));
   blob::save("mesh/ibo.blob", in_mesh->geometry.ibo);
   blob::save("mesh/vbo.blob", in_mesh->geometry.vbo);
   blob::save("mesh/image0.blob", in_mesh->images[0].buffer);
   blob::save("mesh/image1.blob", in_mesh->images[1].buffer);
   blob::save("mesh/image2.blob", in_mesh->images[2].buffer);
   blob::save("mesh/image3.blob", in_mesh->images[3].buffer);
}
void load_mesh(fw::Mesh* out_mesh)
{
   blob::Buffer<fw::Mesh> test = {};
   memset(out_mesh, 0, sizeof(fw::Mesh));
   blob::load("mesh/mesh.blob", test);
   *out_mesh = *test.data;
   blob::load("mesh/ibo.blob", out_mesh->geometry.ibo);
   blob::load("mesh/vbo.blob", out_mesh->geometry.vbo);
   blob::load("mesh/image0.blob", out_mesh->images[0].buffer);
   blob::load("mesh/image1.blob", out_mesh->images[1].buffer);
   blob::load("mesh/image2.blob", out_mesh->images[2].buffer);
   blob::load("mesh/image3.blob", out_mesh->images[3].buffer);
}

void vulkan_sample::run()
{
   commandline::parse();

   log::scope vulkan_sample("vulkan_sample", true);
   
   m_graphics->register_shader("fullscreen", "shaders/spv/fullscreen.vert.spv", shader::e_vertex);
   m_graphics->register_shader("shared", "shaders/spv/shared.vert.spv", shader::e_vertex);
   m_graphics->register_shader("pbr", "shaders/spv/pbr.frag.spv", shader::e_fragment);
   m_graphics->register_shader("unlit", "shaders/spv/unlit.frag.spv", shader::e_fragment);

   auto* tri_verts = initdata::geometry::tri_verts.data();
   unsigned int tri_verts_count = initdata::geometry::tri_verts.size();

   auto* tri_indices = initdata::geometry::tri_indices.data();
   unsigned int tri_indices_count = initdata::geometry::tri_indices.size();

   Mesh swapchain_mesh = {{{tri_verts, tri_verts_count}, {tri_indices, tri_indices_count}}, {}, {}, {hash::string("swapchain")}, {}};;
   swapchain_mesh.material.shaders[fw::shader::e_vertex] = hash::string("fullscreen");
   swapchain_mesh.material.shaders[fw::shader::e_fragment] = hash::string("unlit");
   
   std::vector<Mesh> meshes; std::vector<Image> images;
   std::vector<Mesh> in_meshes; std::vector<Image> in_images;
   float model_scale = 1.0;
   {
      log::scope topic("timer", true);
      log::timer timer("load model");
      loadmodel("../../../../glTF-Sample-Assets/Models/Sponza/glTF/Sponza.gltf", meshes, images); model_scale = 0.02;
      // loadmodel("../../../../glTF-Sample-Assets/Models/SciFiHelmet/glTF/SciFiHelmet.gltf", meshes, images);
   }
   for(Mesh& mesh : meshes)
   {
      mesh.passes = {hash::string("pbr")};
      mesh.material.shaders[fw::shader::e_vertex] = {hash::string("shared")};
      mesh.material.shaders[fw::shader::e_fragment] = {hash::string("pbr")};
      mesh.transform = mat4x4f::scaled(model_scale, model_scale, model_scale);
   }
   {
      {
	 log::scope topic("timer", true);
	 log::timer timer("save mesh");
	 save_mesh(&meshes[2]);
      }
      {
	 log::scope topic("timer", true);
	 log::timer timer("load mesh");
	 load_mesh(&meshes[2]);
      }
   }
   
   float time = 0;
   int shademode = 0;
   camera cam;
   fw::Light light; light.position = vec3f(-1.0f, 5.0f, 1.0f);
   light.intensity = 0.01;
   enum cam_mode {cam_linear, cam_swoop, cam_circle, cam_free, cam_locked, cam_cycle} cmode = cam_cycle;
   int cam_num = 0;
   float cam_rot_offset = 0;
   float cam_dist_offset = 0;
   if(params::get_value("camera", cam_num, 0))
   {
      log::debug("camera mode: {}", cam_num);
      cmode = (cam_mode)cam_num;
   }
   bool cam_toggling = false;
   bool shade_toggling = false;
   bool model_toggling = false;
   int model_id = 0;
   
   ImGui::StyleColorsDark();
   
   auto io = ImGui::GetIO();
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

   while (m_window->update())
   {
      bool wants_shade = m_input->isKeyPressed(input::e_shademode);
      bool wants_cmode = m_input->isKeyPressed(input::e_respawn);
      bool wants_model = m_input->isKeyPressed(input::e_nextmodel);
      ImGui::NewFrame();
      {
	 if(ImGui::Begin("vulkan_sample"))
	 {
	    const char* smodestr = "default";
	    const char* cmodestr = "default";
	    switch(shademode)
	    {
	       case 0: smodestr = "pbr";               break;
	       case 1: smodestr = "texture uvs";       break;
	       case 2: smodestr = "texture albedo";    break;
	       case 3: smodestr = "vertex normals";    break;
	       case 4: smodestr = "texture normals";   break;
	       case 5: smodestr = "world normals";     break;
	       case 6: smodestr = "texture roughness"; break;
	       case 7: smodestr = "texture metallic";  break;
	       case 8: smodestr = "texture ao";        break;
	    }
	    switch(cmode)
	    {
	       case 0: cmodestr = "cam_linear"; break;
	       case 1: cmodestr = "cam_swoop"; break;
	       case 2: cmodestr = "cam_circle"; break;
	       case 3: cmodestr = "cam_free"; break;
	       case 4: cmodestr = "cam_locked"; break;
	       case 5: cmodestr = "cam_cycle"; break;
	    }
	    if (ImGui::Button("next shademode")) wants_shade = true;
	    ImGui::SameLine();
	    ImGui::Text("shademode = %s", smodestr);
	    if (ImGui::Button("next cammode")) wants_cmode = true;
	    ImGui::SameLine();
	    ImGui::Text("cam_mode = %s", cmodestr);
	    if (ImGui::Button("next model")) wants_model = true;
	    ImGui::SameLine();
	    ImGui::Text("model = %d", model_id);
	 }
	 ImGui::End();
      }
      ImGui::Render();
      ImDrawData* ui_drawdata = ImGui::GetDrawData();

      float alpha = 1.0-(cos(time*0.5f)+1.0f)*0.5f;
      if (alpha == 0 && cmode == cam_cycle)
      {
	 cam_num = (cam_num+1)%cam_cycle;
	 log::debug("camera mode: {}", cam_num);
	 cam.m_pitchDegrees = 0;
	 cam.m_headingDegrees = 0;
	 time = 0;
      }
      
      if (wants_cmode && !cam_toggling)
      {
	 int dir = m_input->isKeyPressed(input::e_shift) ? -1 : 1;
	 cmode = (cam_mode)((((int)cmode)+dir) % ((int)cam_cycle));
	 log::debug("user camera mode: {}, {}, {}", cam_num, ((int)cmode+dir), (int)cam_cycle);
	 cam_toggling = true;
	 // todo: make this loop going backwards.
	 cam_num = cmode % cam_cycle;
	 if(cmode != cam_locked && cmode != cam_free)
	 {
	    cam.m_pitchDegrees = 0;
	    cam.m_headingDegrees = 0;
	 }
	 time = 0;
      }
      else if(!wants_cmode && cam_toggling) { cam_toggling = false; }

      if(!shade_toggling && wants_shade)
      {
	 shade_toggling = true;
	 int dir = m_input->isKeyPressed(input::e_shift) ? -1 : 1;
	 shademode = (shademode+dir) % 9;
	 if(shademode < 0) shademode = 8;
	 log::debug("toggle shading: {}", shademode);
	 m_graphics->set_shademode(shademode);
      }
      else if(!wants_shade && shade_toggling) { shade_toggling = false; }

      if(!model_toggling && wants_model)
      {
	 model_toggling = true;
	 int dir = m_input->isKeyPressed(input::e_shift) ? -1 : 1;
	 model_id = (model_id+dir) % meshes.size();
	 if(model_id < 0) model_id = meshes.size()-1;
	 log::debug("toggle meshes: {}", model_id);
      }
      else if(!wants_model && model_toggling) { model_toggling = false; }
      float dt = (1.0f/60.0f);
      switch(cam_num)
      {
	 case cam_free:
	 {
	    // note: IsWindowHovered will assert if you don't use anywindow outwith a window being.
	    if(!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
	    {
	       float dx, dy;
	       m_input->mouseDelta(dx, dy);
	       cam.changeHeading(dx);
	       cam.changePitch(-dy);
	    }
	    const float speed = 10.0;
	    float forward = 0.0;
	    float right = 0.0;
	    right += m_input->isKeyPressed(input::e_right)?speed*dt:0.0;
	    right -= m_input->isKeyPressed(input::e_left)?speed*dt:0.0;
	    forward += m_input->isKeyPressed(input::e_up)?speed*dt:0.0;
	    forward -= m_input->isKeyPressed(input::e_down)?speed*dt:0.0;
	    
	    cam.changeForwardVelocity(-forward);
	    cam.changeStrafeVelocity(right);
	    
	    light.position = cam.m_position;
	    light.intensity = 100.0;

	    break;
	 }
	 case cam_locked: break; // don't do anything.
	 case cam_linear: // note: this is to verify fixing below doesn't break linear view normals.
	    cam.setPosition({0, 1+50*(1.0f-alpha), 0});
	    cam.m_pitchDegrees = 90;
	    break;
	 case cam_swoop: // note: this exposes incorrect normals across the ground quad at 45 degrees
	    cam.setPosition({0, 1+15*(1.0f-alpha),-25*alpha});
	    cam.m_pitchDegrees = 90*(1.0f-alpha);
	    break;
	 case cam_circle: // note: rotate around the scene.
	    const float cam_dist = 2.0f+cam_dist_offset;
	    const float height = 0.5f;
	    const float cam_time = (time*0.1f+cam_rot_offset);
	    cam.setPosition({cam_dist*sin(cam_time), height, -cam_dist*cos(cam_time)});
	    cam.m_headingDegrees = (360.0/(PI*2.0))*cam_time;
	    light.position = cam.m_position;
	    light.position.j = light.position.j;
	    light.intensity = 100*alpha+0.0001;
	    if(m_input->isKeyPressed(input::e_right)) cam_rot_offset += 0.01;
	    if(m_input->isKeyPressed(input::e_left)) cam_rot_offset -= 0.01;
	    if(m_input->isKeyPressed(input::e_down)) cam_dist_offset += 0.01;
	    if(m_input->isKeyPressed(input::e_up)) cam_dist_offset -= 0.01;
	    break;
      }
      // light.intensity = 2.0f*alpha;
      cam.update();
      // todo: you can't isolate model 0.
      for(size_t i = 0; i < meshes.size(); i++) { if(i == (size_t)model_id || !model_id) { m_graphics->getRenderer()->visit(&meshes[i]); } }
      m_graphics->getRenderer()->visit(&cam);
      m_graphics->getRenderer()->visit(&light);
      m_graphics->getRenderer()->visit(ui_drawdata);
      m_graphics->getRenderer()->visit(&swapchain_mesh);

      m_graphics->render();
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
      time += (1.0f/60.0f);
      io.Framerate = 60;
      m_input->update();
      if(m_input->isKeyPressed(input::e_quit)) break;
   }

   for(auto image : images) { delete[] image.buffer.data; }
}

void vulkan_sample::shutdown()
{
   log::scope vulkan_sample("vulkan_sample");

   m_graphics->shutdown();
   m_window->shutdown();
   blob::shutdown();
   log::debug("vulkan_sample finished.");
}
