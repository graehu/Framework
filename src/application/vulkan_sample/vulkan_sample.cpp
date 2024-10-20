#include "vulkan_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../window/window.h"
#include "../../graphics/graphics.h"
#include <array>
#include <cstdint>

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

const std::vector<Vertex> triangle = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}}
    };

void vulkan_sample::run()
{
   commandline::parse();
   log::scope vulkan_sample("vulkan_sample", true);
   m_graphics->register_shader("triangle", "shaders/triangle.vert.spv", shader::e_vertex);
   m_graphics->register_shader("triangle", "shaders/triangle.frag.spv", shader::e_fragment);
   std::array<uint32_t,3> ibo = {0, 1, 2};
   Mesh mesh = {triangle.data(), triangle.size(), ibo.data(), {}};
   mesh.mat[fw::shader::e_vertex] = fw::hash::string("triangle");
   mesh.mat[fw::shader::e_fragment] = fw::hash::string("triangle");
   m_graphics->getRenderer()->visit(&mesh);
   
}
void vulkan_sample::shutdown()
{
   m_graphics->shutdown();
   m_window->shutdown();
}
