#pragma once
#include "../graphics.h"

namespace fwvulkan
{
   struct DrawHandle
   {
      static const int max_draws = 128;
      // todo: owner should also be a handle.
      fw::Mesh* owner = nullptr;
      int vb_handle = -1;
      int ib_handle = -1;
      int im_handles[fw::Mesh::max_images] = {};
      int pi_handle = -1;
      int ds_handle = -1;
   };
   struct VBHandle
   {
      VkBuffer vb;
      VkDeviceMemory vb_mem;
      size_t len;
   };
   // index buffers
   struct IBHandle
   {
      VkBuffer ib;
      VkDeviceMemory ib_mem;
      size_t len;
   };
   struct IMHandle
   {
      VkImage image;
      VkImageView view;
      VkDeviceMemory image_mem;
      size_t width, height;
   };
   struct SamHandle
   {
      VkSampler sampler;
   };
   struct SharedUniforms
   {
      mat4x4f model[DrawHandle::max_draws];
      mat4x4f view;
      mat4x4f proj;
      vec3f light;
      float light_intensity;
      vec3f cam_pos;
      unsigned int shademode;
   };
   // todo: consider renaming to shared.
   struct SharedPushConstants
   {
      uint32_t id;
   };
}
   
