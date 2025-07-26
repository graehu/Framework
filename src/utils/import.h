#pragma once

namespace fw
{
   namespace import
   {
      bool init();
      bool gltf(std::vector<fw::Image*>& in_images, std::vector<Mesh*>& in_meshes, const char* in_path);
      bool shutdown();
   }
}
