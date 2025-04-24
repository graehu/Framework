// todo fill.
#pragma once

namespace fwvulkan
{
   // todo: we shouldn't expose these, but have destruction calls inside the pbr layer.
   extern VkDescriptorPool g_pbr_descriptor_pool;
   extern std::vector<VkDescriptorSet> g_pbr_descriptor_sets;
   extern std::vector<VkDescriptorSet> g_fullscreen_descriptor_sets;
   extern VkDescriptorSetLayout g_pbr_descriptor_set_layout;
   extern VkDevice g_logical_device;
   extern int g_used_pbr_descriptors;
   extern int g_used_fullscreen_descriptors;
   namespace buffers
   {
      // fwd dec
      void SetDescriptorImage(VkImageView image_view, std::vector<VkDescriptorSet> image_sets, unsigned int dst_binding);
      int CreateImageHandle(const unsigned int* image, size_t width, size_t height, size_t bits);
      void InitPBRDescriptors();
      
      namespace descriptor_binds {
	 const unsigned int albedo = 0;
	 const unsigned int roughness = 1;
	 const unsigned int normal = 2;
	 const unsigned int ao = 3;
      }
      void SetPBRDescriptorAlbedo(VkImageView image_view, std::vector<VkDescriptorSet> albedo_sets);
      void SetPBRDescriptorMetallicRoughness(VkImageView image_view, std::vector<VkDescriptorSet> rough_sets);
      void SetPBRDescriptorNormal(VkImageView image_view, std::vector<VkDescriptorSet> normal_sets);
      void SetPBRDescriptorAO(VkImageView image_view, std::vector<VkDescriptorSet> ao_sets);
      void CreatePBRDescriptorSets();
      VkDescriptorPool CreatePBRDescriptorPool();
   }
   namespace pipeline
   {
      VkPipelineLayoutCreateInfo GetPBRPipelineLayout();
   }
}    
