#pragma once

#include "iRenderVisitor.h"
#include "../utils/hasher.h"

#include "../types/vec2f.h"
#include "../types/vec3f.h"
#include "../types/mat4x4f.h"
#include <array>
#include <vector>

// data loading and unloading is done dynamically.
// just try rendering something and it'll load all the needed assests.
namespace fw
{
   
   namespace shader
   {
      enum type {  e_vertex, e_hull, e_domain, e_geomtry, e_fragment, e_task, e_mesh, e_compute, e_count };
   }
   typedef std::array<hash::string, shader::e_count> Material;
   typedef std::vector<hash::string> PassList;
   struct Vertex { vec3f position; vec3f color; vec2f uv; };
   struct VertexArray { const Vertex* data; size_t len; };
   struct IndexArray { const uint16_t* data; size_t len; };
   struct Geometry { VertexArray vbo; IndexArray ibo; };
   struct Image { const unsigned int* data; int width = 0; int height = 0; };
   struct Mesh
   {
      // todo: arbitrary limit for ease of construction atm.
      static const unsigned int max_images = 4;
      Geometry geometry;
      Image images[max_images];
      Material material;
      PassList passes;
      mat4x4f transform;
   };
}
class graphics
{
public:
  virtual int init(void) = 0; //initializing gl and what not for whatever window system is implemented
  virtual int render(void) = 0; //this'll have flipping the buffers
   virtual bool register_shader(fw::hash::string/*name*/, const char*/*path*/, fw::shader::type/*type*/)  { return 0; } // being lazy here, don't want to have to implement this for everything
  virtual bool register_pass(fw::hash::string) {return 0;}
  virtual int shutdown(void) = 0; //shutsdown the graphics engine
  virtual int update(void) = 0; //this is currently useless.
  virtual iRenderVisitor* getRenderer(void) = 0; //passes back the renderer

  static graphics* graphicsFactory(void);

};
