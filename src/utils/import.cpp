#include <vector>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <mutex>
#include "../graphics/graphics.h"
#include "import.h"
#include "blob.h"
#include "filesystem.h"
#include "log/log.h"

#include <execution>
// note: this requires linking ttb (i.e. -lttb) on linux.

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
// 
// todo: this will need different pragmas for windows.
// #pragma warning (push)
// #pragma warning (disable : 4200)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include "tiny_gltf.h"
#pragma GCC diagnostic pop
// #pragma warning (pop)

using namespace fw;
void load_gltf(const char* modelpath, std::vector<Mesh>& out_meshes, std::vector<Image>& out_images)
{
   log::scope topic("importer", true);
   log::debug("loading model: {}", modelpath);
   using namespace tinygltf;

   Model model;
   TinyGLTF loader;
   std::string err;
   std::string warn;

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
   struct ImageCpStruct
   {
      fw::Image* image = nullptr;
      unsigned char* source = nullptr;
      size_t size = 0;
   };
   std::vector<ImageCpStruct> CpImages(model.images.size());
   
   for(unsigned int i = 0; i < model.images.size(); i++)
   {
      auto& image = model.images[i];
      CpImages[i].image = &out_images[i];
      CpImages[i].source = image.image.data();
      CpImages[i].size = image.image.size();
      out_images[i] = {{}, image.width, image.height, image.component*image.bits};
      log::debug("image info: {}/{}: {}, {}x{}, comp: {}, bits: {}, type: {}", (void*)image.image.data(), image.image.size(), image.name.c_str(), image.width, image.height, image.component, image.bits, image.pixel_type);
   }
   std::for_each(std::execution::par, std::begin(CpImages), std::end(CpImages), [](ImageCpStruct& in)
   {
      unsigned int* image_data = new unsigned int[in.size];
      memcpy(image_data, in.source, in.size);
      in.image->buffer.data = image_data;
      in.image->buffer.len = in.size/sizeof(unsigned int);
      fw::hash_image(*in.image);
      
   });
   log::debug("meshes: {}", model.meshes.size());
   out_meshes.reserve(model.meshes.size());
   std::mutex m;
   // todo: this isn't safe to do in std::execution::par for some reason, look into it.
   std::for_each(std::execution::seq, std::begin(model.meshes), std::end(model.meshes), [&](auto& mesh)
   // for (auto& mesh : model.meshes)
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
	    out_mesh.material.flags.alpha = std::strcmp(mat.alphaMode.c_str(), "OPAQUE") != 0;
	    
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
	 std::lock_guard<std::mutex> guard(m);
	 out_meshes.push_back(out_mesh);
      }
   });
}


namespace fw
{
   namespace import
   {
      struct FileHashEntry
      {
	 hash::string filepath;
	 hash::u32 contenthash;
      };
      std::vector<FileHashEntry> filehashes;
      void load_filehashes()
      {
	 blob::asset<FileHashEntry> fhb;
	 blob::miscbank.load("filehashes.blob", fhb);
	 filehashes.resize(fhb.len);
	 for(int i = 0; i < (int)fhb.len; i++)
	 {
	    filehashes[i] = *(fhb.data+i);
	 }
	 blob::miscbank.free(fhb);
      }
      void save_filehashes()
      {
	 blob::asset<FileHashEntry> fhb = {{}, filehashes.data(), filehashes.size()};
	 blob::miscbank.save("filehashes.blob", fhb);
      }
      bool update_filehashes(FileHashEntry entry)
      {
	 bool ret = false;
	 for(auto hash : filehashes)
	 {
	    if(hash.filepath == entry.filepath)
	    {
	       ret = hash.contenthash == entry.contenthash;
	       if(!ret)
	       {
		  hash.contenthash = entry.contenthash;
		  save_filehashes();
	       }
	       return ret;
	    }
	 }
	 filehashes.push_back(entry);
	 save_filehashes();
	 return ret;
      }
      void save_meshes(blob::asset<fw::Mesh> in_meshes, const char* in_name)
      {
	 // explitily serialise each buffer, then load like below.
	 // handle images etc next.
	 for (int i = 0; i < (int)in_meshes.len; i++)
	 {
#define macro(fmtstr) fmt::format(fmtstr, in_name, i).c_str()
	    fw::filesystem::makedirs(macro("{}/meshes/{}"));
      
	    blob::asset<fw::Mesh> mb = {{}, in_meshes.data+i, 1};
	    blob::miscbank.save(macro("{}/meshes/{}/mesh.blob"), mb);
	    blob::miscbank.save(macro("{}/meshes/{}/ibo.blob"), mb.data->geometry.ibo);
	    blob::miscbank.save(macro("{}/meshes/{}/vbo.blob"), mb.data->geometry.vbo);
#undef macro
	 }
      }

      void save_images(blob::asset<fw::Image> in_images, const char* in_name)
      {
	 // explitily serialise each buffer, then load like below.
	 // handle images etc next.
	 for (int i = 0; i < (int)in_images.len; i++)
	 {
#define macro(fmtstr) fmt::format(fmtstr, in_name, i).c_str()
	    fw::filesystem::makedirs(macro("{}/images/{}"));
      
	    blob::asset<fw::Image> ib = {{}, in_images.data+i, 1};
	    blob::miscbank.save(macro("{}/images/{}/image.blob"), ib);
	    blob::miscbank.save(macro("{}/images/{}/ibo.blob"), ib.data->buffer);
#undef macro
	 }
      }

      void load_meshes(std::vector<Mesh*>& out_meshes, const char* in_name)
      {
#define macro(fmtstr) fmt::format(fmtstr, in_name).c_str()
	 int num_meshes = fw::filesystem::countdirs(macro("{}/meshes/"));
	 out_meshes.resize(num_meshes);
	 log::info("num meshes to load: {}", num_meshes);
#undef macro
#define macro(fmtstr) fmt::format(fmtstr, in_name, i).c_str()
	 for (int i = 0; i < num_meshes; i++)
	 {
	    blob::assetnc<fw::Mesh> mb = {{}, nullptr, 1};
	    blob::miscbank.load(macro("{}/meshes/{}/mesh.blob"), mb);
	    blob::miscbank.load(macro("{}/meshes/{}/ibo.blob"), mb.data->geometry.ibo);
	    blob::miscbank.load(macro("{}/meshes/{}/vbo.blob"), mb.data->geometry.vbo);
	    blob::miscbank.fixup(mb.data->images[0].buffer);
	    blob::miscbank.fixup(mb.data->images[1].buffer);
	    blob::miscbank.fixup(mb.data->images[2].buffer);
	    blob::miscbank.fixup(mb.data->images[3].buffer);
	    out_meshes[i] = mb.data;
	 }
#undef macro
      }

      void load_images(std::vector<fw::Image*>& out_images, const char* in_name)
      {
#define macro(fmtstr) fmt::format(fmtstr, in_name).c_str()
	 int num_images = fw::filesystem::countdirs(macro("{}/images/"));
	 out_images.resize(num_images);
	 log::info("num images to load: {}", num_images);
#undef macro
#define macro(fmtstr) fmt::format(fmtstr, in_name, i).c_str()
	 for (int i = 0; i < num_images; i++)
	 {
	    blob::assetnc<fw::Image> ib = {{}, nullptr, 1};
	    blob::miscbank.load(macro("{}/images/{}/image.blob"), ib);
	    blob::miscbank.load(macro("{}/images/{}/ibo.blob"), ib.data->buffer);
	    out_images[i] = ib.data;
	 }
#undef macro
      }

      void save_scene(std::vector<fw::Image>& in_images, std::vector<Mesh>& in_meshes, const char* in_name)
      {
	 log::scope topic("timer", true);
	 log::timer timer("save scene");
	 save_images({{}, in_images.data(), in_images.size()}, in_name);
	 save_meshes({{}, in_meshes.data(), in_meshes.size()}, in_name);
      }
      void load_scene(std::vector<fw::Image*>& out_images, std::vector<Mesh*>& out_meshes, const char* in_name)
      {
	 log::scope topic("timer", true);
	 log::timer timer("load scene");
	 load_images(out_images, in_name);
	 load_meshes(out_meshes, in_name);
      }
      bool init()
      {
	 assert(blob::miscbank.is_initialised());
	 load_filehashes();
	 return true;
      }
      bool gltf(std::vector<fw::Image*>& in_images, std::vector<Mesh*>& in_meshes, const char* in_path)
      {
	 log::scope topic("timer", true);
	 log::timer timer("load gltf");

	 const char* start = in_path;
	 const char* import = nullptr;
	 while(*start != '\0')
	 {
	    if(*start == '/') import = start+1;
	    start++;
	 }
	 const FileHashEntry filehash = {hash::string(import, strlen(import)), filesystem::filehash(in_path)};
	 if(!update_filehashes(filehash) || !filesystem::exists(import))
	 {
	    log::info("not skipped!");
	    std::vector<fw::Image> images; std::vector<fw::Mesh> meshes;
	    load_gltf(in_path, meshes, images);
	    // todo: set this up in load model?
	    for(Mesh& mesh : meshes)
	    {
	       mesh.passes = {hash::string("pbr")};
	       mesh.material.shaders[fw::shader::e_vertex] = {hash::string("shared")};
	       mesh.material.shaders[fw::shader::e_fragment] = {hash::string("pbr")};
	    }
	    save_scene(images, meshes, import);
	    // todo: delete mesh vbs/ibs, etc?
	    for(auto image : images) { delete[] image.buffer.data; }
	 }
	 load_scene(in_images, in_meshes, import);
	 return true;
      }
      bool shutdown()
      {
	 blob::asset<FileHashEntry> fhb = {{}, filehashes.data(), filehashes.size()};
	 blob::miscbank.free(fhb);
	 return true;
      }
   }
}
