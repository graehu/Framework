#include "vulkan_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../window/window.h"
#include "../../graphics/graphics.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <thread>
#include <vector>
#include "iostream"

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
   {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0, 0}},
   {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1, 0}},
   {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1, 1}},
   {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0, 1}}
};

const std::array<unsigned int, 16> white_image =
{
   0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
   0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
   0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
   0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
};

const std::array<uint16_t, 6> quad_indices = {0, 1, 2, 2, 3, 0};

void loadmodel(const char* modelpath, std::vector<Vertex>& verts, std::vector<uint16_t>& indices, std::vector<Image>& images);

// todo: add pass dependencies.
// todo: add pass framebuffer blending/compositing.
// todo: add basic pbr shaders.

void vulkan_sample::run()
{
   commandline::parse();

   log::scope vulkan_sample("vulkan_sample", true);

   m_graphics->register_shader("triangle", "shaders/triangle.vert.spv", shader::e_vertex);
   m_graphics->register_shader("triangle", "shaders/triangle.frag.spv", shader::e_fragment);
   m_graphics->register_shader("uv_triangle", "shaders/uv_triangle.frag.spv", shader::e_fragment);
   
   Mesh quad = {{{quad_verts.data(), quad_verts.size()}, {quad_indices.data(), quad_indices.size()}}, {}, {}, {"swapchain"}, {}};
   quad.material[fw::shader::e_vertex] = fw::hash::string("triangle");
   quad.material[fw::shader::e_fragment] = fw::hash::string("uv_triangle");

   Mesh quad2 = {{{quad_verts.data(), quad_verts.size()}, {quad_indices.data(), quad_indices.size()}}, {{white_image.data(), 4, 4}}, {}, {"swapchain"}, {}};
   quad2.material[fw::shader::e_vertex] = fw::hash::string("triangle");
   quad2.material[fw::shader::e_fragment] = fw::hash::string("uv_triangle");
   quad2.transform = mat4x4f::scaled(10, 10, 10) * mat4x4f::rotated(deg2rag(90), 0, 0) * mat4x4f::translated(0, 1, 0);
   
   std::vector<Vertex> model_verts;   std::vector<uint16_t> model_indices; std::vector<Image> images;
   loadmodel("../../../libs/tinygltf/models/Cube/Cube.gltf", model_verts, model_indices, images);
   Mesh model = {
      {{model_verts.data(), model_verts.size()}, {model_indices.data(),model_indices.size()}},
      {images[0], images[1]},
      {}, {"swapchain"}, {}
   };

   model.material[fw::shader::e_vertex] = fw::hash::string("triangle");
   model.material[fw::shader::e_fragment] = fw::hash::string("triangle");
   float time = 0;

   while (m_window->update())
   {
      m_graphics->getRenderer()->visit(&model);
      m_graphics->getRenderer()->visit(&quad);
      m_graphics->getRenderer()->visit(&quad2);
      m_graphics->render();
      model.transform =  mat4x4f::translated(2, 0, 0)*mat4x4f::rotated(0, time, 0);
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

// todo: this really shouldn't live in vulkan_sample.cpp

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#include "tiny_gltf.h"

void loadmodel(const char* modelpath, std::vector<Vertex>& verts, std::vector<uint16_t>& indices, std::vector<Image>& images)
{
   log::debug("loading model: {}", modelpath);
   using namespace tinygltf;

   Model model;
   TinyGLTF loader;
   std::string err;
   std::string warn;
   const float scale = 1.0f;

   bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, modelpath);
   //bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

   if (!warn.empty())
   {
      log::warn("warning: {}", warn.c_str());
   }
   
   if (!err.empty())
   {
      log::error("error: {}", err.c_str());
      return;
   }

   if (!ret)
   {
      log::error("Failed to parse {}", modelpath);
      return;
   }

   log::debug("images: {}", model.images.size());
   for(auto& image : model.images)
   {
      log::debug("image info: {}, {}, {}", image.name.c_str(), image.width, image.height);
      images.push_back({(const unsigned int*)image.image.data(), image.width, image.height});
   }
   
   for (auto& primitive : model.meshes[0].primitives)
   {
      const tinygltf::Accessor& pos_accessor = model.accessors[primitive.attributes["POSITION"]];
      const tinygltf::BufferView& pos_bufferView = model.bufferViews[pos_accessor.bufferView];
      const tinygltf::Buffer& pos_buffer = model.buffers[pos_bufferView.buffer];

      const tinygltf::Accessor& uv_accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
      const tinygltf::BufferView& uv_bufferView = model.bufferViews[uv_accessor.bufferView];
      const tinygltf::Buffer& uv_buffer = model.buffers[pos_bufferView.buffer];

      const float* positions = reinterpret_cast<const float*>(&pos_buffer.data[pos_bufferView.byteOffset + pos_accessor.byteOffset]);
      const float* uvs = reinterpret_cast<const float*>(&uv_buffer.data[uv_bufferView.byteOffset + uv_accessor.byteOffset]);
      // switch(uv_accessor.type)
      // {
      // 	 case TINYGLTF_TYPE_VEC2: //TINYGLTF_TYPE_VEC2
      // 	 {
      // 	    switch (uv_accessor.componentType)
      // 	    {
      // 	       case TINYGLTF_COMPONENT_TYPE_FLOAT:
      // 	       {
      // 		  log::debug("this float");
      // 		  break;
      // 	       }
      // 	       case TINYGLTF_COMPONENT_TYPE_DOUBLE :
      // 	       {
      // 		  log::debug("that double");
      // 		  break;
      // 	       }
      // 	    }
      // 	    break;
      // 	 }
      // 	 default:
      // 	 {
      // 	    log::debug("unknown type");
      // 	 }
      // }

      for (size_t i = 0; i < pos_accessor.count; ++i) {
	 verts.push_back({
	       { positions[i * 3 + 0]*scale,  positions[i * 3 + 1]*scale,  positions[i * 3 + 2]*scale },
	       // note: debug colours
	       // {float((i+0)%3), float((i+1)%3), float((1+2)%3)} 
	       {1.0f, 1.0f, 1.0f}, { uvs[i * 2 + 0], uvs[i * 2 + 1] }
	    });
      }
      
      const tinygltf::Accessor& idx_accessor = model.accessors[primitive.indices];
      const tinygltf::BufferView& idx_bufferView = model.bufferViews[idx_accessor.bufferView];
      const tinygltf::Buffer& idx_buffer = model.buffers[idx_bufferView.buffer];
      const uint16_t *data = reinterpret_cast<const uint16_t *>(&idx_buffer.data[idx_bufferView.byteOffset + idx_accessor.byteOffset]);
      indices.resize(idx_accessor.count);
      
      for (size_t i = 0; i < idx_accessor.count; ++i) {
	 indices[i] = data[i];
      }
   }
}
