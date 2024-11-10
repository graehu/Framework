#include "vulkan_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../window/window.h"
#include "../../graphics/graphics.h"
#include <array>
#include <cstdint>
#include <thread>

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
   m_window->init(m_width, m_height, m_name);

   fw::log::topics::add("graphics");
   m_graphics = graphics::graphicsFactory();
   m_graphics->init();
}

const std::vector<Vertex> triangle_1 = {
   {{ 0.0f,-0.5f}, {1.0f, 0.0f, 0.0f}},
   {{ 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
   {{-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}}
};

const std::vector<Vertex> triangle_2 = {
   {{( 0.0f+1.0f)*.5f, (-0.5f)*.5f}, {1.0f, 0.0f, 0.0f}},
   {{( 0.5f+1.0f)*.5f, ( 0.5f)*.5f}, {0.0f, 1.0f, 0.0f}},
   {{(-0.5f+1.0f)*.5f, ( 0.5f)*.5f}, {1.0f, 0.0f, 1.0f}}
};

void vulkan_sample::run()
{
   commandline::parse();
   log::scope vulkan_sample("vulkan_sample", true);
   m_graphics->register_shader("triangle", "shaders/triangle.vert.spv", shader::e_vertex);
   m_graphics->register_shader("triangle", "shaders/triangle.frag.spv", shader::e_fragment);
   
   std::array<uint32_t,3> ibo = {0, 1, 2};
   Mesh mesh1 = {triangle_1.data(), triangle_1.size(), ibo.data(), {}, {"default"}};
   mesh1.mat[fw::shader::e_vertex] = fw::hash::string("triangle");
   mesh1.mat[fw::shader::e_fragment] = fw::hash::string("triangle");
   
   Mesh mesh2 = {triangle_2.data(), triangle_2.size(), ibo.data(), {}, {"doot"}};
   mesh2.mat[fw::shader::e_vertex] = fw::hash::string("triangle");
   mesh2.mat[fw::shader::e_fragment] = fw::hash::string("triangle");
   
   while (m_window->update())
   {
      m_graphics->register_pass("doot");
      m_graphics->getRenderer()->visit(&mesh1);
      m_graphics->getRenderer()->visit(&mesh2);
      m_graphics->render();
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
   }
}

void vulkan_sample::shutdown()
{
   log::scope vulkan_sample("vulkan_sample");
   m_graphics->shutdown();
   m_window->shutdown();
   log::debug("vulkan_sample finished.");
}
