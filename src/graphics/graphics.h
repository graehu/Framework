#pragma once

#include "iRenderVisitor.h"
#include "../utils/hasher.h"
#include "../utils/blob.h"

#include "../types/vec2f.h"
#include "../types/vec3f.h"
#include "../types/mat4x4f.h"
#include <array>
#include <vector>

namespace fw
{
   namespace shader
   {
      enum type {  e_vertex, e_hull, e_domain, e_geomtry, e_fragment, e_task, e_mesh, e_compute, e_count };
   }
   struct Material
   {
      std::array<hash::u32, shader::e_count> shaders = {};
      struct {
	 bool alpha : 1 = false;
	 size_t pad : 7 = 0;
      } flags;
   };
   // todo: this would be better as a u32 bit field with bits mapping to predefined pass hashes.
   static const unsigned int max_passes = 16;
   typedef std::array<hash::u32, max_passes> PassList;
   struct Vertex { vec3f position; vec3f normal; vec3f color; vec2f uv; };
   typedef blob::Buffer<Vertex> VertexBuffer;
   typedef blob::Buffer<uint32_t> IndexBuffer;
   typedef blob::Buffer<unsigned int> ImageBuffer;
   struct Geometry { VertexBuffer vbo; IndexBuffer ibo; };
   struct Image { ImageBuffer buffer; int width = 0; int height = 0; int bits = 0; };
   struct Mesh
   {
      // todo: arbitrary limit for ease of construction atm.
      // note: this awkwardly ties into how many images we allow on the drawhandler atm.
      static const unsigned int max_images = 4;
      Geometry geometry = {};
      Image images[max_images] = {};
      Material material = {};
      PassList passes = {};
      mat4x4f transform = {};
   };
   static_assert(sizeof(fw::Mesh) == 376, "fw::Mesh is serialisable, the layout matters.");
   
   struct Light
   {
      vec3f position;
      float intensity = 1.0f;
   };
   inline bool hash_image(Image& image)
   {
      if (image.buffer.head.hash == 0)
      {
	 image.buffer.head.hash = hash::hash_buffer((const char*)image.buffer.data, sizeof(int)*image.buffer.len);//image.width*image.height*(image.bits/8));
	 return true;
      }
      return false;
   }
}
class graphics
{
public:
   virtual int init(void) = 0; //initializing gl and what not for whatever window system is implemented
   virtual int render(void) = 0; //this'll have flipping the buffers
   virtual bool register_shader(fw::hash::string/*name*/, const char*/*path*/, fw::shader::type/*type*/)  { return 0; } // being lazy here, don't want to have to implement this for everything
   virtual bool register_material(fw::Material /*material*/)  { return 0; } // being lazy here, don't want to have to implement this for everything
   virtual bool register_pass(fw::hash::string) {return 0;}
   virtual void set_shademode(int /*shademode*/) { } // lazy
   virtual int shutdown(void) = 0; //shutsdown the graphics engine
   virtual int update(void) = 0; //this is currently useless.
   virtual iRenderVisitor* getRenderer(void) = 0; //passes back the renderer
   static graphics* graphicsFactory(void);
};
namespace fw
{
   namespace initdata
   {
      namespace images
      {
	 // magenta checkcard
	 const std::array<unsigned int, 16> missing_data =
	 {
	    0x00000000, 0xffff00ff, 0x00000000, 0xffff00ff,
	    0xffff00ff, 0x00000000, 0xffff00ff, 0x00000000,
	    0x00000000, 0xffff00ff, 0x00000000, 0xffff00ff,
	    0xffff00ff, 0x00000000, 0xffff00ff, 0x00000000,
	 };
	 // todo: need a way to precalculate the hash and force it into the g_im_map.
	 const Image missing = { {missing_data.data(), 16}, 4, 4, 32 };
	 const std::array<unsigned int, 16> white_data =
	 {
	    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	 };
	 // todo: need a way to precalculate the hash and force it into the g_im_map.
	 const Image white = { {white_data.data(), 16}, 4, 4, 32 };
	 const std::array<unsigned int, 16> black_data =
	 {
	    0x00000000, 0x00000000, 0x00000000, 0x00000000,
	    0x00000000, 0x00000000, 0x00000000, 0x00000000,
	    0x00000000, 0x00000000, 0x00000000, 0x00000000,
	    0x00000000, 0x00000000, 0x00000000, 0x00000000,
	 };
	 // todo: need a way to precalculate the hash and force it into the g_im_map.
	 const Image black = { {black_data.data(), 16}, 4, 4, 32 };
	 const std::array<unsigned int, 16> grey_data =
	 {
	    0x08080808, 0x08080808, 0x08080808, 0x08080808,
	    0x08080808, 0x08080808, 0x08080808, 0x08080808,
	    0x08080808, 0x08080808, 0x08080808, 0x08080808,
	    0x08080808, 0x08080808, 0x08080808, 0x08080808,
	 };
	 // todo: need a way to precalculate the hash and force it into the g_im_map.
	 const Image grey = { {black_data.data(), 16}, 4, 4, 32 };
      }
      namespace geometry
      {
	 const std::vector<Vertex> quad_verts = {
	    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0, 0}},
	    {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0, 1}},
	    {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1, 1}},
	    {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1, 0}},
	 };
	 
	 const std::array<uint32_t, 6> quad_indices = {0, 1, 2, 2, 3, 0};

	 const std::vector<Vertex> tri_verts = {
	    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0, 0}},
	    {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0, 1}},
	    {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1, 1}}
	 };
	 
	 const std::array<uint32_t, 3> tri_indices = {0, 1, 2};
      }
   }
}    
