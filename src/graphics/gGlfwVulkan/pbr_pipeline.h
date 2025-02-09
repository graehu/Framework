// todo fill.
#pragma once

// we can get rid of this easily enough.
#include <map>
namespace fwvulkan
{
   namespace initdata
   {
      namespace images
      {
	 extern const std::array<unsigned int, 16> argb;
      }
   }

   /* struct IMHandler; */
   extern std::map<uint32_t, struct IMHandle> g_im_map;         // readonly/sampled.
   extern VkDescriptorPool g_pbr_descriptor_pool;
   extern std::vector<VkDescriptorSet> g_pbr_descriptor_sets;
   extern VkDescriptorSetLayout g_pbr_descriptor_set_layout;
   extern VkDevice g_logical_device;
   namespace buffers
   {
      // fwd dec
      void SetDescriptorImage(VkImageView image_view, std::vector<VkDescriptorSet> image_sets, unsigned int dst_binding);
      int CreateImageHandle(const unsigned int* image, size_t width, size_t height);
      void InitPBRDescriptors();
      
      namespace descriptor_binds {
	 const unsigned int albedo = 0;
	 const unsigned int roughness = 1;
      }
      void SetDescriptorAlbedo(VkImageView image_view, std::vector<VkDescriptorSet> albedo_sets);
      void SetDescriptorRoughness(VkImageView image_view, std::vector<VkDescriptorSet> rough_sets);
      void CreatePBRDescriptorSets();
      VkDescriptorPool CreatePBRDescriptorPool();
   }
   namespace pipeline
   {
      VkPipelineLayoutCreateInfo GetPBRPipelineLayout();
   }
}    
