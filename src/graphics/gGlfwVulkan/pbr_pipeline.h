// todo fill.
#pragma once

namespace fwvulkan
{
   extern VkDescriptorPool g_pbr_descriptor_pool;
   extern std::vector<VkDescriptorSet> g_pbr_descriptor_sets;
   extern VkDescriptorSetLayout g_pbr_descriptor_set_layout;
   extern VkDevice g_logical_device;
   extern int g_used_pbr_descriptors;
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
      void SetPBRDescriptorAlbedo(VkImageView image_view, std::vector<VkDescriptorSet> albedo_sets);
      void SetPBRDescriptorRoughness(VkImageView image_view, std::vector<VkDescriptorSet> rough_sets);
      void CreatePBRDescriptorSets();
      VkDescriptorPool CreatePBRDescriptorPool();
   }
   namespace pipeline
   {
      VkPipelineLayoutCreateInfo GetPBRPipelineLayout();
   }
}    
