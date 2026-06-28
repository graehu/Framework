#pragma once

namespace fw
{
   namespace importer
   {
      bool init();
      bool load_gltf(std::vector<fw::Image*>& in_images, std::vector<Mesh*>& in_meshes, const char* in_path);
      bool unload_gltf(std::vector<fw::Image*>& in_images, std::vector<Mesh*>& in_meshes);
      bool load_scene_zip(std::vector<fw::Image*>& in_images, std::vector<Mesh*>& in_meshes, const char* in_path);
      bool save_scene_zip(const char* in_name);
      bool invalidate_cache();
      bool shutdown();
   }
}
