#pragma once

#include "iRenderVisitor.h"
#include "../utils/hasher.h"

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
   typedef std::array<hash::string, shader::e_count> Material;
   typedef std::vector<hash::string> PassList;
   struct Vertex { vec3f position; vec3f normal; vec3f color; vec2f uv; };
   struct VertexArray { const Vertex* data; size_t len; };
   struct IndexArray { const uint32_t* data; size_t len; };
   struct Geometry { VertexArray vbo; IndexArray ibo; };
   struct Image { const unsigned int* data = nullptr; int width = 0; int height = 0; int bits = 0;};
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
   struct Light
   {
      vec3f position;
      float intensity = 1.0f;
   };
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
	 const std::array<unsigned int, 16> missing =
	 {
	    0x00000000, 0xffff00ff, 0x00000000, 0xffff00ff,
	    0xffff00ff, 0x00000000, 0xffff00ff, 0x00000000,
	    0x00000000, 0xffff00ff, 0x00000000, 0xffff00ff,
	    0xffff00ff, 0x00000000, 0xffff00ff, 0x00000000,
	 };
	 // magenta checkcard
	 const std::array<unsigned int, 16> white =
	 {
	    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	 };
	 const std::array<unsigned int, 16> black =
	 {
	    0x00000000, 0x00000000, 0x00000000, 0x00000000,
	    0x00000000, 0x00000000, 0x00000000, 0x00000000,
	    0x00000000, 0x00000000, 0x00000000, 0x00000000,
	    0x00000000, 0x00000000, 0x00000000, 0x00000000,
	 };
	 const std::array<unsigned int, 16> grey =
	 {
	    0x08080808, 0x08080808, 0x08080808, 0x08080808,
	    0x08080808, 0x08080808, 0x08080808, 0x08080808,
	    0x08080808, 0x08080808, 0x08080808, 0x08080808,
	    0x08080808, 0x08080808, 0x08080808, 0x08080808,
	 };
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
      }
   }
}    
