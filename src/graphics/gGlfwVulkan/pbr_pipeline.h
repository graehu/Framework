// todo fill.
#pragma once
#include "vulkan_types.h"
namespace fw { struct Image; struct Mesh;};
namespace fwvulkan
{
   namespace pbr
   {
      void Init();
      void SetDescriptorAlbedo(VkImageView image_view, std::vector<VkDescriptorSet> albedo_sets);
      void SetDescriptorMetallicRoughness(VkImageView image_view, std::vector<VkDescriptorSet> rough_sets);
      void SetDescriptorNormal(VkImageView image_view, std::vector<VkDescriptorSet> normal_sets);
      void SetDescriptorAO(VkImageView image_view, std::vector<VkDescriptorSet> ao_sets);
      VkPipelineLayoutCreateInfo GetPipelineLayout();
      VkDescriptorSet& GetDescriptorSet(unsigned int handle);
      DrawHandle visit(fw::Mesh* _mesh);
      void Reset();
      void Shutdown();
   }
   namespace fullscreen
   {
      void Init();
      VkDescriptorSet& GetDescriptorSet(unsigned int handle);
      DrawHandle visit(fw::Mesh* _mesh);
      void Reset();
      void Shutdown();
   }
}    
