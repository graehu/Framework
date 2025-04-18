#include <cassert>
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
   out_images.resize(model.images.size());
   for(int i = 0; i < model.images.size(); i++)
   // for(auto& image : model.images)
   {
      auto& image = model.images[i];
      log::debug("image info: {}/{}: {}, {}x{}, comp: {}, bits: {}, type: {}", (void*)image.image.data(), image.image.size(), image.name.c_str(), image.width, image.height, image.component, image.bits, image.pixel_type);
      unsigned int* image_data = new unsigned int[image.image.size()];
      memcpy(image_data, image.image.data(), image.image.size());
      out_images[i] = {(const unsigned int*)image_data, image.width, image.height, image.component*image.bits};
      // out_images.push_back({(const unsigned int*)image_data, image.width, image.height, image.component*image.bits});
   }

   log::debug("meshes: {}", model.meshes.size());
   for (auto& mesh : model.meshes)
   {
      log::debug("primatives: {}", mesh.primitives.size());
      for (auto& primitive : mesh.primitives)
      {
	 fw::Mesh out_mesh;
	 // todo: support more triangle modes
	 assert(primitive.mode == TINYGLTF_MODE_TRIANGLES);
	 
	 const tinygltf::Accessor& pos_accessor = model.accessors[primitive.attributes["POSITION"]];
	 const tinygltf::BufferView& pos_bufferView = model.bufferViews[pos_accessor.bufferView];
	 const tinygltf::Buffer& pos_buffer = model.buffers[pos_bufferView.buffer];
	 int pos_stride = pos_accessor.ByteStride(pos_bufferView);

	 const tinygltf::Accessor& norm_accessor = model.accessors[primitive.attributes["NORMAL"]];
	 const tinygltf::BufferView& norm_bufferView = model.bufferViews[norm_accessor.bufferView];
	 const tinygltf::Buffer& norm_buffer = model.buffers[norm_bufferView.buffer];
	 int norm_stride = norm_accessor.ByteStride(norm_bufferView);
	 // todo: support TEXCOORD_1 etc. asserts below.
	 const tinygltf::Accessor& uv_accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
	 const tinygltf::BufferView& uv_bufferView = model.bufferViews[uv_accessor.bufferView];
	 const tinygltf::Buffer& uv_buffer = model.buffers[pos_bufferView.buffer];
	 int uv_stride = uv_accessor.ByteStride(uv_bufferView);
	 
	 const tinygltf::Accessor& idx_accessor = model.accessors[primitive.indices];
	 const tinygltf::BufferView& idx_bufferView = model.bufferViews[idx_accessor.bufferView];
	 const tinygltf::Buffer& idx_buffer = model.buffers[idx_bufferView.buffer];
	 int idx_stride = idx_accessor.ByteStride(idx_bufferView);

	 const float* positions = reinterpret_cast<const float*>(&pos_buffer.data[pos_bufferView.byteOffset + pos_accessor.byteOffset]);
	 const float* normals = reinterpret_cast<const float*>(&norm_buffer.data[norm_bufferView.byteOffset + norm_accessor.byteOffset]);
	 const float* uvs = reinterpret_cast<const float*>(&uv_buffer.data[uv_bufferView.byteOffset + uv_accessor.byteOffset]);
	 
	 const uint32_t* data32 = reinterpret_cast<const uint32_t *>(&idx_buffer.data[idx_bufferView.byteOffset + idx_accessor.byteOffset]);
	 const uint16_t* data16 = reinterpret_cast<const uint16_t *>(&idx_buffer.data[idx_bufferView.byteOffset + idx_accessor.byteOffset]);
	 const uint8_t* data8 = reinterpret_cast<const uint8_t *>(&idx_buffer.data[idx_bufferView.byteOffset + idx_accessor.byteOffset]);

	 log::debug("verts: {}, mode: {}, pos: {}, norm: {}, uv: {}, idx: {}", pos_accessor.count, primitive.mode, pos_stride, norm_stride, uv_stride, idx_stride);
	 
	 switch(uv_accessor.type)
	 {
	    case TINYGLTF_TYPE_VEC2: //TINYGLTF_TYPE_VEC2
	    {
	       switch (uv_accessor.componentType)
	       {
		  case TINYGLTF_COMPONENT_TYPE_FLOAT:
		  {
		     log::debug("UVs: float");
		     break;
		  }
		  case TINYGLTF_COMPONENT_TYPE_DOUBLE :
		  {
		     log::debug("UVs: double");
		     // todo: support double type uvs
		     assert(false);
		     break;
		  }
	       }
	       break;
	    }
	    default:
	    {
	       log::debug("unknown type");
	    }
	 }
	 
	 Vertex* verts = new Vertex[pos_accessor.count];
	 for (size_t i = 0; i < pos_accessor.count; ++i)
	 {
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
      
	 uint32_t* indices = new uint32_t[idx_accessor.count];
	 for (size_t i = 0; i < idx_accessor.count; ++i)
	 {
	    if(idx_stride == 1) indices[i] = data8[i];
	    else if(idx_stride == 2) indices[i] = data16[i];
	    else if(idx_stride == 4) indices[i] = data32[i];
	    
	 }
	 out_mesh.geometry.ibo.data = indices;
	 out_mesh.geometry.ibo.len = idx_accessor.count;

	 int material_id = primitive.material;
	 if(material_id != -1)
	 {
	    tinygltf::Material& mat = model.materials[material_id];
	    const char* name = model.materials[material_id].name.c_str();
	    log::debug("material: {}, {}", material_id, name);
	    
	    int tex_id = mat.pbrMetallicRoughness.baseColorTexture.index;
	    int coord_id = mat.pbrMetallicRoughness.baseColorTexture.texCoord;
	    if(tex_id != -1)
	    {
	       assert(coord_id == 0);
	       log::debug("base tex: {}, coords: {}", tex_id, coord_id);
	       out_mesh.images[0] = out_images[model.textures[tex_id].source];
	    }
	    
	    tex_id = mat.pbrMetallicRoughness.metallicRoughnessTexture.index;
	    coord_id = mat.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;
	    if(tex_id != -1)
	    {
	       assert(coord_id == 0);
	       log::debug("met-rough tex: {}, coords: {}", tex_id, coord_id);
	       out_mesh.images[1] = out_images[model.textures[tex_id].source];
	    }
	    
	    tex_id = mat.normalTexture.index;
	    coord_id = mat.normalTexture.texCoord;
	    if(tex_id != -1)
	    {
	       assert(coord_id == 0);
	       log::debug("normal tex: {}, coords: {}", tex_id, coord_id);
	       out_mesh.images[2] = out_images[model.textures[tex_id].source];
	    }
	    
	    tex_id = mat.occlusionTexture.index;
	    coord_id = mat.occlusionTexture.texCoord;
	    if(mat.occlusionTexture.index != -1)
	    {
	       assert(coord_id == 0);
	       log::debug("ao tex: {}, coords: {}", tex_id, coord_id);
	       out_mesh.images[3] = out_images[model.textures[tex_id].source];
	    }
	 }
	 out_meshes.push_back(out_mesh);
      }
   }
}
