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
}

const std::vector<Vertex> quad_verts = {
   {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0, 0}},
   {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0, 1}},
   {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1, 1}},
   {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1, 0}},
};

const std::vector<Vertex> quad_verts2 = {
   {{-0.5f,  0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0, 0}},
   {{-0.5f,  0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0, 1}},
   {{ 0.5f,  0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1, 1}},
   {{ 0.5f,  0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1, 0}},
};

const std::array<unsigned int, 16> white_image =
{
   0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
   0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
   0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
   0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
};

const std::array<uint16_t, 6> quad_indices = {0, 1, 2, 2, 3, 0};


// todo: add pass dependencies.
// todo: add pass framebuffer blending/compositing.
// todo: add basic pbr shaders.
// todo: add input to cycle cameras
// todo: add user input camera

void vulkan_sample::run()
{
   commandline::parse();

   log::scope vulkan_sample("vulkan_sample", true);

   m_graphics->register_shader("triangle", "shaders/triangle.vert.spv", shader::e_vertex);
   m_graphics->register_shader("triangle", "shaders/triangle.frag.spv", shader::e_fragment);
   m_graphics->register_shader("uv_triangle", "shaders/uv_triangle.frag.spv", shader::e_fragment);
   
   Mesh quad = {{{quad_verts2.data(), quad_verts2.size()}, {quad_indices.data(), quad_indices.size()}}, {}, {}, {"swapchain"}, {}};
   quad.material[fw::shader::e_vertex] = fw::hash::string("triangle");
   quad.material[fw::shader::e_fragment] = fw::hash::string("uv_triangle");

   Mesh quad2 = {{{quad_verts.data(), quad_verts.size()}, {quad_indices.data(), quad_indices.size()}}, {{white_image.data(), 4, 4}}, {}, {"swapchain"}, {}};
   quad2.material[fw::shader::e_vertex] = fw::hash::string("triangle");
   quad2.material[fw::shader::e_fragment] = fw::hash::string("triangle");

   
   std::vector<Vertex> model_verts;   std::vector<uint16_t> model_indices; std::vector<Image> images;
   loadmodel("../../../libs/tinygltf/models/Cube/Cube.gltf", model_verts, model_indices, images);

   Mesh model = {
      {{model_verts.data(), model_verts.size()}, {model_indices.data(),model_indices.size()}},
      // todo: make this less of a hack.
      {images[0], images[1]},
      {}, {"swapchain"}, {}
   };
   model.material[fw::shader::e_vertex] = fw::hash::string("triangle");
   model.material[fw::shader::e_fragment] = fw::hash::string("triangle");

   Mesh model2 = model;
   float time = 0;
   quad.transform = mat4x4f::translated(1, 0, -2);
   model2.transform = mat4x4f::translated(4, 1, 4);
   quad2.transform = mat4x4f::scaled(10, 10, 10)  * mat4x4f::rotated(deg2rad(90), 0, 0)*mat4x4f::translated(0, -1, 0);
   camera cam;
   fw::Light light; light.position = vec3f(0.0f, 0.5f);
   enum cam_mode {cam_linear, cam_swoop, cam_circle, cam_cycle} cmode = cam_cycle;
   int cam_num = 0;
   if(params::get_value("camera", cam_num, 0))
   {
      log::debug("camera mode: {}", cam_num);
      cmode = (cam_mode)cam_num;
   }
   while (m_window->update())
   {
      model.transform = mat4x4f::scaled(.25,.25,.25)*mat4x4f::rotated(deg2rad(-90), deg2rad(-90), 0)*mat4x4f::translated(1, 0, 0)*mat4x4f::rotated(0, time*2.0f, 0)*mat4x4f::translated(0, 0, 0);
      model2.transform = mat4x4f::rotated(0, time*2.0f, 0)*mat4x4f::translated(4, 1, 4);
      float alpha = 1.0-(cos(time*0.5f)+1.0f)*0.5f;
      if (alpha == 0 && cmode == cam_cycle)
      {
	 cam_num = (cam_num+1)%cam_cycle;
	 log::debug("camera mode: {}", cam_num);
	 cam.m_pitchDegrees = 0;
	 cam.m_headingDegrees = 0;
	 time = 0;
      }
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
	    cam.setPosition({10*sin(time), 3, -10*cos(time)});
	    cam.m_headingDegrees = -(360.0/(PI*2.0))*time;
	    // cam.m_pitchDegrees = 15; // todo: this shows that the view matrix is wrong or something.
	    break;
      }
      light.intensity = 2.0f*alpha;
      cam.update();
      m_graphics->getRenderer()->visit(&model);
      m_graphics->getRenderer()->visit(&quad);
      m_graphics->getRenderer()->visit(&quad2);
      m_graphics->getRenderer()->visit(&model2);
      m_graphics->getRenderer()->visit(&cam);
      m_graphics->getRenderer()->visit(&light);
      m_graphics->render();
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
      time += 1.0f/60.0f;
   }
}

void vulkan_sample::shutdown()
{
   log::scope vulkan_sample("vulkan_sample");
   m_graphics->shutdown();
   m_window->shutdown();
   log::debug("vulkan_sample finished.");
}
