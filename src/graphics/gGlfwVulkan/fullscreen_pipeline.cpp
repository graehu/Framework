#include "vulkan/vulkan.hpp"
#include "fullscreen_pipeline.h"
#include "../graphics.h"
#include "vulkan_types.h"
#include "../../utils/log/log.h"
#include "pipeline_shared.h"

using namespace fw;

namespace fwvulkan
{
   namespace fullscreen
   {
      VkDescriptorPool g_descriptor_pool;
      std::vector<VkDescriptorSet> g_descriptor_sets;
      VkDescriptorSetLayout g_descriptor_set_layout;
      int g_used_descriptors = 0;
      const unsigned int g_num_textures = 4;
      // todo: this is too many descriptors and they're wrongly named.
      namespace descriptor_binds
      {
	 const unsigned int albedo = 0;
	 const unsigned int roughness = 1;
	 const unsigned int normal = 2;
	 const unsigned int ao = 3;
      }
      void SetDescriptorAlbedo(VkImageView image_view, std::vector<VkDescriptorSet> albedo_sets)
      {
	 log::debug("SetFullscreenDescriptorAlbedo: {}", size_t(image_view));
	 buffers::SetDescriptorImage(image_view, albedo_sets, descriptor_binds::albedo);
      }
      void SetDescriptorMetallicRoughness(VkImageView image_view, std::vector<VkDescriptorSet> rough_sets)
      {
	 log::debug("SetFullscreenDescriptorMetallicRoughness: {}", size_t(image_view));
	 buffers::SetDescriptorImage(image_view, rough_sets, descriptor_binds::roughness);
      }
      void SetDescriptorNormal(VkImageView image_view, std::vector<VkDescriptorSet> normal_sets)
      {
	 log::debug("SetFullscreenDescriptorNormal: {}", size_t(image_view));
	 buffers::SetDescriptorImage(image_view, normal_sets, descriptor_binds::normal);
      }
      void SetDescriptorAO(VkImageView image_view, std::vector<VkDescriptorSet> ao_sets)
      {
	 log::debug("SetFullscreenDescriptorAO: {}", size_t(image_view));
	 buffers::SetDescriptorImage(image_view, ao_sets, descriptor_binds::ao);
      }
      void CreateDescriptorSets()
      {
	 log::debug("CreateFullscreenDescriptorSets");
	 
	 std::vector<VkDescriptorSetLayout> layouts(DrawHandle::max_draws, g_descriptor_set_layout);
	 VkDescriptorSetAllocateInfo alloc_info{};
	 
	 alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	 alloc_info.descriptorPool = g_descriptor_pool;
	 alloc_info.descriptorSetCount = DrawHandle::max_draws;
	 alloc_info.pSetLayouts = layouts.data();
	 g_descriptor_sets.resize(DrawHandle::max_draws);
	 
	 if (vkAllocateDescriptorSets(g_logical_device, &alloc_info, g_descriptor_sets.data()) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to allocate descriptor sets!");
	 }

	 fw::Image white = initdata::images::white;
	 fw::Image black = initdata::images::black;
	 
	 buffers::CreateImageHandle(white);
	 buffers::CreateImageHandle(black);
	    
	 SetDescriptorAlbedo(g_im_map[white.buffer.head.hash].view, g_descriptor_sets);
	 SetDescriptorMetallicRoughness(g_im_map[white.buffer.head.hash].view, g_descriptor_sets);
	 SetDescriptorNormal(g_im_map[black.buffer.head.hash].view, g_descriptor_sets);
	 SetDescriptorAO(g_im_map[white.buffer.head.hash].view, g_descriptor_sets);
      }
      VkPipelineLayoutCreateInfo GetPipelineLayout()
      {
	 static std::array<VkDescriptorSetLayout, 2> layouts;
	 layouts = {g_shared_descriptor_set_layout, g_descriptor_set_layout};
	 static VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
	 pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	 pipeline_layout_ci.setLayoutCount = layouts.size();
	 pipeline_layout_ci.pSetLayouts = layouts.data();
	 
	 static VkPushConstantRange push_constant = {};
	 push_constant.offset = 0;
	 push_constant.size = sizeof(SharedPushConstants);
	 push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	 pipeline_layout_ci.pPushConstantRanges = &push_constant;
	 pipeline_layout_ci.pushConstantRangeCount = 1;
	 
	 return pipeline_layout_ci;
      }
      VkDescriptorPool CreateDescriptorPool()
      {
	 VkDescriptorPoolSize pool_sizes[g_num_textures] = {};
	 for(unsigned int i = 0; i < g_num_textures; i++)
	 {
	    pool_sizes[i].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	    pool_sizes[i].descriptorCount = DrawHandle::max_draws;
	 }

	 VkDescriptorPoolCreateInfo pool_ci{};
	 pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	 pool_ci.poolSizeCount = g_num_textures;
	 pool_ci.pPoolSizes = &pool_sizes[0];
	 pool_ci.maxSets = DrawHandle::max_draws;

	 VkDescriptorPool pool;
	 if (vkCreateDescriptorPool(g_logical_device, &pool_ci, nullptr, &pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
	 }
	 return pool;
      }
      void InitDescriptors()
      {
	 VkDescriptorType types[g_num_textures] = { };
	 VkShaderStageFlags stages[g_num_textures] = { };
	 for(unsigned int i = 0; i < g_num_textures; i++) { types[i] = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; }
	 for(unsigned int i = 0; i < g_num_textures; i++) { stages[i] = VK_SHADER_STAGE_FRAGMENT_BIT; }
	 g_descriptor_set_layout = buffers::CreateDescriptorSetLayout(types, stages, g_num_textures);
	 g_descriptor_pool = CreateDescriptorPool();
	 
	 CreateDescriptorSets();
      }
      VkDescriptorSet& GetDescriptorSet(unsigned int handle)
      {
	 return g_descriptor_sets[handle];
      }
      void Init()
      {
	 InitDescriptors();
      }
      void Reset() { g_used_descriptors = 0; }
      void Shutdown()
      {
	 Reset();
	 vkDestroyDescriptorPool(g_logical_device, g_descriptor_pool, nullptr);
	 vkDestroyDescriptorSetLayout(g_logical_device, g_descriptor_set_layout, nullptr);
      }
      DrawHandle visit(fw::Mesh *_mesh)
      {
	 log::debug("fullscreen draw descriptors: {}/{}", g_used_descriptors, g_descriptor_sets.size());
	 DrawHandle drawhandle = { _mesh,
	    buffers::CreateVertexBufferHandle(_mesh->geometry.vbo.data, _mesh->geometry.vbo.len),
	    buffers::CreateIndexBufferHandle(_mesh->geometry.ibo.data, _mesh->geometry.ibo.len),
	    {},
	    // todo: this should be "create all pipeline variants"
	    // ----: then we store draws against pipelines / passes, or something. :)
	    // todo: have this read the material to decide the pipeline layout.
	    pipeline::CreatePipelineVariants(_mesh->material, GetPipelineLayout()),
	    0
	 };

	 drawhandle.ds_handle = g_used_descriptors++;
	 assert(size_t(g_used_descriptors) <  g_descriptor_sets.size());
	 std::vector<VkDescriptorSet> set(1, {g_descriptor_sets[drawhandle.ds_handle]});
	 SetDescriptorAlbedo(renderpass::GetPassImageView("ui"), set);
	 SetDescriptorMetallicRoughness(renderpass::GetPassImageView("pbr"), set);
	 SetDescriptorNormal(renderpass::GetPassImageView("pbr"), set);
	 SetDescriptorAO(renderpass::GetPassImageView("pbr"), set); 
	 return drawhandle;
      }
   }
}
