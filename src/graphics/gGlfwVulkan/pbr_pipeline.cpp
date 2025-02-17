#include "vulkan/vulkan.hpp"
#include "pbr_pipeline.h"
#include "../graphics.h"
#include "vulkan_types.h"
#include "../../utils/log/log.h"

using namespace fw;

namespace fwvulkan
{
   VkDescriptorPool g_pbr_descriptor_pool;
   std::vector<VkDescriptorSet> g_pbr_descriptor_sets;
   VkDescriptorSetLayout g_pbr_descriptor_set_layout;
   int g_used_pbr_descriptors = 0;
   // put these in vulkan_types.h?
   extern VkDescriptorSetLayout g_shared_descriptor_set_layout;
   extern std::map<uint32_t, struct IMHandle> g_im_map;
   
   namespace buffers
   {
      // fwd dec
      void SetDescriptorImage(VkImageView image_view, std::vector<VkDescriptorSet> image_sets, unsigned int dst_binding);
      VkDescriptorSetLayout CreateDescriptorSetLayout(VkDescriptorType* types, VkShaderStageFlags* stage_flags, int num);
      void SetPBRDescriptorAlbedo(VkImageView image_view, std::vector<VkDescriptorSet> albedo_sets)
      {
	  log::debug("SetPBRDescriptorAlbedo: {}", size_t(image_view));
	 SetDescriptorImage(image_view, albedo_sets, descriptor_binds::albedo);
      }
      void SetPBRDescriptorRoughness(VkImageView image_view, std::vector<VkDescriptorSet> rough_sets)
      {
	 log::debug("SetPBRDescriptoRoughness: {}", size_t(image_view));
	 SetDescriptorImage(image_view, rough_sets, descriptor_binds::roughness);
      }
      void CreatePBRDescriptorSets()
      {
	 log::debug("CreatePBRDescriptorSets");
	 std::vector<VkDescriptorSetLayout> layouts(DrawHandle::max_draws, g_pbr_descriptor_set_layout);
	 VkDescriptorSetAllocateInfo alloc_info{};
	 alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	 alloc_info.descriptorPool = g_pbr_descriptor_pool;
	 alloc_info.descriptorSetCount = DrawHandle::max_draws;
	 alloc_info.pSetLayouts = layouts.data();
	 g_pbr_descriptor_sets.resize(DrawHandle::max_draws);
	 
	 if (vkAllocateDescriptorSets(g_logical_device, &alloc_info, g_pbr_descriptor_sets.data()) != VK_SUCCESS)
	 {
            throw std::runtime_error("failed to allocate descriptor sets!");
	 }
	 
	 auto im_handle = CreateImageHandle(initdata::images::missing.data(), 4, 4);
	 buffers::SetPBRDescriptorAlbedo(g_im_map[im_handle].view, g_pbr_descriptor_sets);
	 buffers::SetPBRDescriptorRoughness(g_im_map[im_handle].view, g_pbr_descriptor_sets);
      }
      VkDescriptorPool CreatePBRDescriptorPool()
      {
	 VkDescriptorPoolSize pool_sizes[2];
	 pool_sizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	 pool_sizes[0].descriptorCount = DrawHandle::max_draws;
         pool_sizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	 pool_sizes[1].descriptorCount = DrawHandle::max_draws;

	 VkDescriptorPoolCreateInfo pool_ci{};
	 pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	 pool_ci.poolSizeCount = 2;
	 pool_ci.pPoolSizes = &pool_sizes[0];
	 pool_ci.maxSets = DrawHandle::max_draws;

	 VkDescriptorPool pool;
	 if (vkCreateDescriptorPool(g_logical_device, &pool_ci, nullptr, &pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
	 }
	 return pool;
      }
      void InitPBRDescriptors()
      {
	 VkDescriptorType types[] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE };
	 VkShaderStageFlags stages[] = { VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
	 g_pbr_descriptor_set_layout = buffers::CreateDescriptorSetLayout(types, stages, 2);
	 g_pbr_descriptor_pool = buffers::CreatePBRDescriptorPool();
	 buffers::CreatePBRDescriptorSets();
      }
   }
   namespace pipeline
   {
      VkPipelineLayoutCreateInfo GetPBRPipelineLayout()
      {
	 static std::array<VkDescriptorSetLayout, 2> layouts;
	 layouts = {g_shared_descriptor_set_layout, g_pbr_descriptor_set_layout};
	 static VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
	 pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	 pipeline_layout_ci.setLayoutCount = layouts.size();
	 pipeline_layout_ci.pSetLayouts = layouts.data();
	 
	 static VkPushConstantRange push_constant = {};
	 push_constant.offset = 0;
	 push_constant.size = sizeof(DefaultPushConstants);
	 push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	 pipeline_layout_ci.pPushConstantRanges = &push_constant;
	 pipeline_layout_ci.pushConstantRangeCount = 1;
	 
	 return pipeline_layout_ci;
      }
   }
}    
