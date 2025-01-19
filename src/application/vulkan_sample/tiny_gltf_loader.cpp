#include <vector>
#include "../../utils/log/log.h"
#include "../../graphics/graphics.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#include "tiny_gltf.h"
using namespace fw;
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

      const tinygltf::Accessor& norm_accessor = model.accessors[primitive.attributes["NORMAL"]];
      const tinygltf::BufferView& norm_bufferView = model.bufferViews[norm_accessor.bufferView];
      const tinygltf::Buffer& norm_buffer = model.buffers[norm_bufferView.buffer];

      const tinygltf::Accessor& uv_accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
      const tinygltf::BufferView& uv_bufferView = model.bufferViews[uv_accessor.bufferView];
      const tinygltf::Buffer& uv_buffer = model.buffers[pos_bufferView.buffer];

      const float* positions = reinterpret_cast<const float*>(&pos_buffer.data[pos_bufferView.byteOffset + pos_accessor.byteOffset]);
      const float* normals = reinterpret_cast<const float*>(&norm_buffer.data[norm_bufferView.byteOffset + norm_accessor.byteOffset]);
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
	       { positions[i * 3 + 0],  positions[i * 3 + 1],  positions[i * 3 + 2] },
	       { normals[i * 3 + 0],  normals[i * 3 + 1],  normals[i * 3 + 2] },
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
