#include "vulkan_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../window/window.h"
#include "../../graphics/graphics.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <thread>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "iostream"
#include "../../graphics/camera/camera.h"
#include "../../input/input.h"

// todo: we don't want to have this as an explicit file like this I don't think.
#include "tiny_gltf_loader.h"

using namespace fw;

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

   fw::log::topics::add("graphics");
   m_graphics = graphics::graphicsFactory();
   m_graphics->init();

   fw::log::topics::add("input");
   m_input = input::inputFactory();
   m_input->init();
}

// todo: add pass dependencies.
// todo: add pass framebuffer blending/compositing.
// todo: add basic pbr shaders.
// todo: add shademode to shader global constants.
// todo: add user input camera

void vulkan_sample::run()
{
   commandline::parse();

   log::scope vulkan_sample("vulkan_sample", true);

   m_graphics->register_shader("shared", "shaders/spv/shared.vert.spv", shader::e_vertex);
   m_graphics->register_shader("pbr", "shaders/spv/pbr.frag.spv", shader::e_fragment);
   m_graphics->register_shader("unlit", "shaders/spv/unlit.frag.spv", shader::e_fragment);

   auto* white_image = initdata::images::white.data();
   
   auto* quad_verts = initdata::geometry::quad_verts.data();
   unsigned int quad_verts_count = initdata::geometry::quad_verts.size();
   
   auto* quad_indices = initdata::geometry::quad_indices.data();
   unsigned int quad_indices_count = initdata::geometry::quad_indices.size();
   
   Mesh quad = {{{quad_verts, quad_verts_count}, {quad_indices, quad_indices_count}}, {}, {}, {"swapchain"}, {}};
   quad.material[fw::shader::e_vertex] = fw::hash::string("shared");
   quad.material[fw::shader::e_fragment] = fw::hash::string("unlit");

   Mesh quad2 = {{{quad_verts, quad_verts_count}, {quad_indices, quad_indices_count}}, {{white_image, 4, 4, 32}}, {}, {"swapchain"}, {}};
   quad2.material[fw::shader::e_vertex] = fw::hash::string("shared");
   quad2.material[fw::shader::e_fragment] = fw::hash::string("pbr");

   
   std::vector<Mesh> meshes; std::vector<Image> images;
   // loadmodel("../../../libs/tinygltf/models/Cube/Cube.gltf", meshes, images);
   // loadmodel("../../../../glTF-Sample-Assets/Models/Sponza/glTF/Sponza.gltf", meshes, images);
   loadmodel("../../../../glTF-Sample-Assets/Models/SciFiHelmet/glTF/SciFiHelmet.gltf", meshes, images);
   // loadmodel("../../../../glTF-Sample-Assets/Models/SheenChair/glTF/SheenChair.gltf", meshes, images);
   // loadmodel("../../../../glTF-Sample-Assets/Models/ABeautifulGame/glTF/ABeautifulGame.gltf", meshes, images);
   // loadmodel("../../../../CopyCat/Project/GamePlay/Characters/CopyCat/Bodies/CopyCat.gltf", meshes, images);
   
   for(Mesh& mesh : meshes)
   {
      mesh.passes = {"swapchain"};
      mesh.material[fw::shader::e_vertex] = fw::hash::string("shared");
      mesh.material[fw::shader::e_fragment] = fw::hash::string("pbr");
      // mesh.transform = mat4x4f::translated(0, -.5, -0.1)*mat4x4f::scaled(2, 2, 2);
      // mesh.transform = mat4x4f::scaled(40, 40, 40);
      // mesh.transform = mat4x4f::scaled(.02, .02, .02);
   }

   float time = 0;
   quad.transform =  mat4x4f::rotated(deg2rad(75), 0, 0)*mat4x4f::translated(1, 0, -2);
   quad2.transform = mat4x4f::scaled(10, 10, 10)  * mat4x4f::rotated(deg2rad(90), 0, 0) * mat4x4f::translated(0, -1, 0);
   camera cam;
   fw::Light light; light.position = vec3f(-1.0f, 5.0f, 1.0f);
   light.intensity = 0.01;
   enum cam_mode {cam_linear, cam_swoop, cam_circle, cam_cycle} cmode = cam_cycle;
   int cam_num = 0;
   float cam_rot_offset = 0;
   float cam_dist_offset = 0;
   if(params::get_value("camera", cam_num, 0))
   {
      log::debug("camera mode: {}", cam_num);
      cmode = (cam_mode)cam_num;
   }
   bool cam_toggling = false;
   while (m_window->update())
   {
      float alpha = 1.0-(cos(time*0.5f)+1.0f)*0.5f;
      if (alpha == 0 && cmode == cam_cycle)
      {
	 cam_num = (cam_num+1)%cam_cycle;
	 log::debug("camera mode: {}", cam_num);
	 cam.m_pitchDegrees = 0;
	 cam.m_headingDegrees = 0;
	 time = 0;
      }
      bool wants_toggle = m_input->isKeyPressed(input::e_respawn);
      if (wants_toggle && !cam_toggling)
      {
	 cmode = (cam_mode)(((int)cmode+1) % (int)cam_cycle+1);
	 log::debug("user camera mode: {}, {}, {}", cam_num, ((int)cmode+1), (int)cam_cycle);
	 cam_toggling = true;
	 cam_num = cmode%cam_cycle;
	 cam.m_pitchDegrees = 0;
	 cam.m_headingDegrees = 0;
	 time = 0;
      }
      else if (!wants_toggle && cam_toggling) { cam_toggling = false; }
      switch(cam_num)
      {
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
	    cam.m_headingDegrees = -(360.0/(PI*2.0))*cam_time;
	    light.position = cam.m_position;
	    light.position.j = light.position.j;
	    light.intensity = 100*alpha+0.0001;
	    if(m_input->isKeyPressed(input::e_right)) cam_rot_offset += 0.01;
	    if(m_input->isKeyPressed(input::e_left)) cam_rot_offset -= 0.01;
	    if(m_input->isKeyPressed(input::e_down)) cam_dist_offset += 0.01;
	    if(m_input->isKeyPressed(input::e_up)) cam_dist_offset -= 0.01;
	    // light.position.i = -light.position.i;
	    // light.position.k = -light.position.k;
	    // cam.m_pitchDegrees = 15; // todo: this shows that the view matrix is wrong or something.
	    break;
      }
      // light.intensity = 2.0f*alpha;
      cam.update();
      for(Mesh& mesh : meshes) { m_graphics->getRenderer()->visit(&mesh); }
      // m_graphics->getRenderer()->visit(&meshes[0]);
      // m_graphics->getRenderer()->visit(&quad);
      // m_graphics->getRenderer()->visit(&quad2);
      m_graphics->getRenderer()->visit(&cam);
      m_graphics->getRenderer()->visit(&light);
      m_graphics->render();
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
      time += (1.0f/60.0f);
      m_input->update();
      if(m_input->isKeyPressed(input::e_quit)) break;
   }
   for(auto image : images) { delete[] image.data; }
}

void vulkan_sample::shutdown()
{
   log::scope vulkan_sample("vulkan_sample");
   m_graphics->shutdown();
   m_window->shutdown();
   log::debug("vulkan_sample finished.");
}
