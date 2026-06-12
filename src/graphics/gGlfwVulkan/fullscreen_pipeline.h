// todo fill.
#pragma once
#include "vulkan_types.h"
namespace fw { struct Image; struct Mesh; };
namespace fwvulkan
{
   namespace fullscreen
   {
      void Init();
      VkDescriptorSet& GetDescriptorSet(unsigned int handle);
      DrawHandle render(fw::Mesh* _mesh);
      void Reset();
      void Shutdown();
   }
}
