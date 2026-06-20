#pragma once
#include "../graphics2.h"

namespace fwvulkan
{
   struct VBHandle;
   struct IBHandle;
   struct IMHandle;
   struct PipelineHandle;
   struct DrawHandle
   {
      static const int max_draws = 128;
      // todo: owner should also be a handle.
      fw::Mesh* owner = nullptr;
      fw::HandlePtr<VBHandle> vb_handle = {0, 0}; //VBHandle* vb_ptr = nullptr;
      fw::HandlePtr<IBHandle> ib_handle = {0, 0}; //IBHandle* ib_ptr = nullptr;
      fw::HandlePtr<IMHandle> im_handles[fw::Mesh::max_images] = {};
      //IMHandle* im_ptrs[fw::Mesh::max_images] = {};
      fw::HandlePtr<PipelineHandle> pi_handle = {0, 0}; //void* pi_ptr = nullptr;
      int ds_handle = -1; //void* ds_ptr = nullptr;
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

