#include <vector>
#include "../../utils/log/log.h"
#include "../../graphics/graphics.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#include "tiny_gltf.h"
using namespace fw;
void loadmodel(const char* modelpath, std::vector<Mesh>& out_meshes, std::vector<Image>& out_images)
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
      out_images.push_back({(const unsigned int*)image.image.data(), image.width, image.height});
   }
   log::debug("meshes: {}", model.meshes.size());
   for (auto& mesh : model.meshes)
   {
      log::debug("primatives: {}", mesh.primitives.size());
      for (auto& primitive : mesh.primitives)
      {
	 out_meshes.push_back(fw::Mesh());
	 auto& out_mesh = out_meshes.back();
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

	 log::debug("verts: {}", pos_accessor.count);
	 // switch(uv_accessor.type)
	 // {
	 //    case TINYGLTF_TYPE_VEC2: //TINYGLTF_TYPE_VEC2
	 //    {
	 //       switch (uv_accessor.componentType)
	 //       {
	 // 	  case TINYGLTF_COMPONENT_TYPE_FLOAT:
	 // 	  {
	 // 	     log::debug("UVs: float");
	 // 	     break;
	 // 	  }
	 // 	  case TINYGLTF_COMPONENT_TYPE_DOUBLE :
	 // 	  {
	 // 	     log::debug("UVs: double");
	 // 	     break;
	 // 	  }
	 //       }
	 //       break;
	 //    }
	 //    default:
	 //    {
	 //       log::debug("unknown type");
	 //    }
	 // }
	 Vertex* verts = new Vertex[pos_accessor.count];
	 for (size_t i = 0; i < pos_accessor.count; ++i) {
	    verts[i] = {
		  { positions[i * 3 + 0],  positions[i * 3 + 1],  positions[i * 3 + 2] },
		  { normals[i * 3 + 0],  normals[i * 3 + 1],  normals[i * 3 + 2] },
		  // note: debug colours
		  // {float((i+0)%3), float((i+1)%3), float((1+2)%3)} 
		  {1.0f, 1.0f, 1.0f}, { uvs[i * 2 + 0], uvs[i * 2 + 1] }
	       };
	 }
	 out_mesh.geometry.vbo.data = verts;
	 out_mesh.geometry.vbo.len = pos_accessor.count;
      
	 const tinygltf::Accessor& idx_accessor = model.accessors[primitive.indices];
	 const tinygltf::BufferView& idx_bufferView = model.bufferViews[idx_accessor.bufferView];
	 const tinygltf::Buffer& idx_buffer = model.buffers[idx_bufferView.buffer];
	 const uint16_t *data = reinterpret_cast<const uint16_t *>(&idx_buffer.data[idx_bufferView.byteOffset + idx_accessor.byteOffset]);
	 uint16_t* indices = new uint16_t[idx_accessor.count];
      
	 for (size_t i = 0; i < idx_accessor.count; ++i) {
	    indices[i] = data[i];
	 }
	 out_mesh.geometry.ibo.data = indices;
	 out_mesh.geometry.ibo.len = idx_accessor.count;

	 int material_id = primitive.material;
	 if(material_id != -1)
	 {
	    tinygltf::Material& mat = model.materials[material_id];
	    const char* name = model.materials[material_id].name.c_str();
	    log::debug("material: {}", name);

	    if(mat.pbrMetallicRoughness.baseColorTexture.index != -1)
	    {
	       log::debug("base tex: {}", mat.pbrMetallicRoughness.baseColorTexture.index);
	       out_mesh.images[0] = out_images[mat.pbrMetallicRoughness.baseColorTexture.index];
	    }
	    if(mat.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
	    {
	       log::debug("met-rough tex: {}", mat.pbrMetallicRoughness.metallicRoughnessTexture.index);
	       out_mesh.images[1] = out_images[mat.pbrMetallicRoughness.metallicRoughnessTexture.index];
	    }
	    if(mat.normalTexture.index != -1)
	    {
	       log::debug("normal tex: {}", mat.normalTexture.index);
	       out_mesh.images[2] = out_images[mat.normalTexture.index];
	    }
	    if(mat.occlusionTexture.index != -1)
	    {
	       log::debug("ao tex: {}", mat.occlusionTexture.index);
	       out_mesh.images[3] = out_images[mat.occlusionTexture.index];
	    }
	 }
      }
   }
}
