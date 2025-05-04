#include "vulkan/vulkan.hpp"
#include "pbr_pipeline.h"
#include "../graphics.h"
#include "vulkan_types.h"
#include "../../utils/log/log.h"

using namespace fw;

namespace fwvulkan
{

   VkDescriptorPool g_pbr_descriptor_pool;
   VkDescriptorPool g_fullscreen_descriptor_pool;
   std::vector<VkDescriptorSet> g_pbr_descriptor_sets;
   std::vector<VkDescriptorSet> g_fullscreen_descriptor_sets;
   VkDescriptorSetLayout g_pbr_descriptor_set_layout;
   VkDescriptorSetLayout g_fullscreen_descriptor_set_layout;
   int g_used_pbr_descriptors = 0;
   int g_used_fullscreen_descriptors = 0;
   // put these in vulkan_types.h?
   extern VkDevice g_logical_device;
   extern VkDescriptorSetLayout g_shared_descriptor_set_layout;
   extern std::map<uint32_t, struct IMHandle> g_im_map;
   // extern std::map<hash::string, struct PassHandle> g_pass_map;
   extern std::map<fw::hash::string, struct PassHandle> g_pass_map;
   const unsigned int g_pbr_num_textures = 4;
   namespace descriptor_binds {
      const unsigned int albedo = 0;
      const unsigned int roughness = 1;
      const unsigned int normal = 2;
      const unsigned int ao = 3;
   }
   namespace renderpass
   {
      VkImageView GetPassImageView(hash::string passname);
   }
   namespace buffers
   {
      extern int CreateImageHandle(fw::Image& image);
      extern int CreateVertexBufferHandle(const fw::Vertex* vertices, int num_vertices);
      extern int CreateIndexBufferHandle(const uint32_t* indices, int num_indices);
      extern void SetDescriptorImage(VkImageView image_view, std::vector<VkDescriptorSet> image_sets, unsigned int dst_binding);
      extern VkDescriptorSetLayout CreateDescriptorSetLayout(VkDescriptorType* types, VkShaderStageFlags* stage_flags, int num);
      namespace pbr
      {
	 // fwd dec
	 void SetDescriptorAlbedo(VkImageView image_view, std::vector<VkDescriptorSet> albedo_sets)
	 {
	    log::debug("SetPBRDescriptorAlbedo: {}", size_t(image_view));
	    SetDescriptorImage(image_view, albedo_sets, descriptor_binds::albedo);
	 }
	 void SetDescriptorMetallicRoughness(VkImageView image_view, std::vector<VkDescriptorSet> rough_sets)
	 {
	    log::debug("SetPBRDescriptorMetallicRoughness: {}", size_t(image_view));
	    SetDescriptorImage(image_view, rough_sets, descriptor_binds::roughness);
	 }
	 void SetDescriptorNormal(VkImageView image_view, std::vector<VkDescriptorSet> normal_sets)
	 {
	    log::debug("SetPBRDescriptorNormal: {}", size_t(image_view));
	    SetDescriptorImage(image_view, normal_sets, descriptor_binds::normal);
	 }
	 void SetDescriptorAO(VkImageView image_view, std::vector<VkDescriptorSet> ao_sets)
	 {
	    log::debug("SetPBRDescriptorAO: {}", size_t(image_view));
	    SetDescriptorImage(image_view, ao_sets, descriptor_binds::ao);
	 }
	 // todo:: fullscreen descriptor sets need to move from here.
	 void CreateDescriptorSets()
	 {
	    log::debug("CreatePBRDescriptorSets");
	 
	    std::vector<VkDescriptorSetLayout> layouts(DrawHandle::max_draws, g_pbr_descriptor_set_layout);
	    VkDescriptorSetAllocateInfo alloc_info{};
	 
	    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	    alloc_info.descriptorPool = g_pbr_descriptor_pool;
	    alloc_info.descriptorSetCount = DrawHandle::max_draws;
	    alloc_info.pSetLayouts = layouts.data();
	    g_pbr_descriptor_sets.resize(DrawHandle::max_draws);
	    g_fullscreen_descriptor_sets.resize(DrawHandle::max_draws);
	 
	    if (vkAllocateDescriptorSets(g_logical_device, &alloc_info, g_pbr_descriptor_sets.data()) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to allocate descriptor sets!");
	    }
	    std::vector<VkDescriptorSetLayout> fullscreen_layouts(DrawHandle::max_draws, g_fullscreen_descriptor_set_layout);
	    alloc_info.descriptorPool = g_fullscreen_descriptor_pool;
	    alloc_info.pSetLayouts = fullscreen_layouts.data();
	    if (vkAllocateDescriptorSets(g_logical_device, &alloc_info, g_fullscreen_descriptor_sets.data()) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to allocate descriptor sets!");
	    }
	    fw::Image white = initdata::images::white;
	    fw::Image grey = initdata::images::grey;
	    fw::Image black = initdata::images::black;
	    CreateImageHandle(white);
	    CreateImageHandle(grey);
	    CreateImageHandle(black);
	    
	    SetDescriptorAlbedo(g_im_map[white.hash].view, g_pbr_descriptor_sets);
	    SetDescriptorMetallicRoughness(g_im_map[white.hash].view, g_pbr_descriptor_sets);
	    SetDescriptorNormal(g_im_map[black.hash].view, g_pbr_descriptor_sets);
	    SetDescriptorAO(g_im_map[grey.hash].view, g_pbr_descriptor_sets);

	    SetDescriptorAlbedo(g_im_map[white.hash].view, g_fullscreen_descriptor_sets);
	    SetDescriptorMetallicRoughness(g_im_map[white.hash].view, g_fullscreen_descriptor_sets);
	    SetDescriptorNormal(g_im_map[black.hash].view, g_fullscreen_descriptor_sets);
	    SetDescriptorAO(g_im_map[grey.hash].view, g_fullscreen_descriptor_sets);
	 }

      VkDescriptorPool CreatePBRDescriptorPool()
      {
	 VkDescriptorPoolSize pool_sizes[g_pbr_num_textures] = {};
	 for(unsigned int i = 0; i < g_pbr_num_textures; i++)
	 {
	    pool_sizes[i].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	    pool_sizes[i].descriptorCount = DrawHandle::max_draws;
	 }

	 VkDescriptorPoolCreateInfo pool_ci{};
	 pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	 pool_ci.poolSizeCount = g_pbr_num_textures;
	 pool_ci.pPoolSizes = &pool_sizes[0];
	 pool_ci.maxSets = DrawHandle::max_draws;

	 VkDescriptorPool pool;
	 if (vkCreateDescriptorPool(g_logical_device, &pool_ci, nullptr, &pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
	 }
	 return pool;
      }
      }
      void InitPBRDescriptors()
      {
	 VkDescriptorType types[g_pbr_num_textures] = { };
	 VkShaderStageFlags stages[g_pbr_num_textures] = { };
	 for(unsigned int i = 0; i < g_pbr_num_textures; i++) { types[i] = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; }
	 for(unsigned int i = 0; i < g_pbr_num_textures; i++) { stages[i] = VK_SHADER_STAGE_FRAGMENT_BIT; }
	 g_pbr_descriptor_set_layout = buffers::CreateDescriptorSetLayout(types, stages, g_pbr_num_textures);
	 g_fullscreen_descriptor_set_layout = buffers::CreateDescriptorSetLayout(types, stages, g_pbr_num_textures);
	 
	 g_pbr_descriptor_pool = buffers::pbr::CreatePBRDescriptorPool();
	 g_fullscreen_descriptor_pool = buffers::pbr::CreatePBRDescriptorPool();
	 
	 buffers::pbr::CreateDescriptorSets();
      }
   }
   namespace pipeline
   {
      extern int CreatePipelineVariants(Material mat, VkPipelineLayoutCreateInfo pipeline_layout_ci);
      namespace pbr
      {
	 VkPipelineLayoutCreateInfo GetPipelineLayout()
	 {
	    static std::array<VkDescriptorSetLayout, 2> layouts;
	    layouts = {g_shared_descriptor_set_layout, g_pbr_descriptor_set_layout};
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
      }
   }
   namespace pbr
   {
      void Init()
      {
	 buffers::InitPBRDescriptors();
      }
      DrawHandle visit(fw::Mesh *_mesh)
      {
	 log::debug("pbr draw descriptors: {}/{}", g_used_pbr_descriptors, g_pbr_descriptor_sets.size());
	 DrawHandle drawhandle = { _mesh,
	    buffers::CreateVertexBufferHandle(_mesh->geometry.vbo.data, _mesh->geometry.vbo.len),
	    buffers::CreateIndexBufferHandle(_mesh->geometry.ibo.data, _mesh->geometry.ibo.len),
	    {},
	    // todo: this should be "create all pipeline variants"
	    // ----: then we store draws against pipelines / passes, or something. :)
	    // todo: have this read the material to decide the pipeline layout.
	    pipeline::CreatePipelineVariants(_mesh->material, pipeline::pbr::GetPipelineLayout()),
	    0
	 };
      	 drawhandle.ds_handle = g_used_pbr_descriptors++;
	 assert(size_t(g_used_pbr_descriptors) <  g_pbr_descriptor_sets.size());
	 std::vector<VkDescriptorSet> set(1, {g_pbr_descriptor_sets[drawhandle.ds_handle]});
	 for(unsigned int i = 0; i < Mesh::max_images; i++)
	 {
	    if(_mesh->images[i].data == nullptr) continue;
	    // todo: add image type field to fw::Image so we can assign more dynamically than below.
	    // ----: their order inside _mesh->images[i] should be arbitrary.
	    // todo: handle non pbr textures.
	    drawhandle.im_handles[i] = buffers::CreateImageHandle(_mesh->images[i]);
	    // drawhandle.im_handles[i] = buffers::CreateImageHandle(_mesh->images[i].data, _mesh->images[i].width, _mesh->images[i].height, _mesh->images[i].bits);
	    if(i == 0) buffers::pbr::SetDescriptorAlbedo(g_im_map[drawhandle.im_handles[0]].view, set);
	    else if(i == 1) buffers::pbr::SetDescriptorMetallicRoughness(g_im_map[drawhandle.im_handles[1]].view, set);
	    else if(i == 2) buffers::pbr::SetDescriptorNormal(g_im_map[drawhandle.im_handles[2]].view, set);
	    else if(i == 3) buffers::pbr::SetDescriptorAO(g_im_map[drawhandle.im_handles[3]].view, set);
	 }
	 return drawhandle;
      }
      VkDescriptorSet& GetDescriptorSet(unsigned int handle)
      {
	 return g_pbr_descriptor_sets[handle];
      }
      void Reset()
      {
	 g_used_pbr_descriptors = 0;
      }
      void Shutdown()
      {
	 Reset();
	 vkDestroyDescriptorPool(g_logical_device, g_pbr_descriptor_pool, nullptr);
	 vkDestroyDescriptorSetLayout(g_logical_device, g_pbr_descriptor_set_layout, nullptr);
      }
   }
   namespace fullscreen
   {
      VkDescriptorSet& GetDescriptorSet(unsigned int handle)
      {
	 return g_fullscreen_descriptor_sets[handle];
      }
      void Reset()
      {
	 g_used_pbr_descriptors = 0;
      }
      void Shutdown()
      {
	 Reset();
	 vkDestroyDescriptorPool(g_logical_device, g_fullscreen_descriptor_pool, nullptr);
	 vkDestroyDescriptorSetLayout(g_logical_device, g_fullscreen_descriptor_set_layout, nullptr);
      }
      DrawHandle visit(fw::Mesh *_mesh)
      {
	 log::debug("fullscreen draw descriptors: {}/{}", g_used_fullscreen_descriptors, g_fullscreen_descriptor_sets.size());
	 DrawHandle drawhandle = { _mesh,
	    buffers::CreateVertexBufferHandle(_mesh->geometry.vbo.data, _mesh->geometry.vbo.len),
	    buffers::CreateIndexBufferHandle(_mesh->geometry.ibo.data, _mesh->geometry.ibo.len),
	    {},
	    // todo: this should be "create all pipeline variants"
	    // ----: then we store draws against pipelines / passes, or something. :)
	    // todo: have this read the material to decide the pipeline layout.
	    pipeline::CreatePipelineVariants(_mesh->material, pipeline::pbr::GetPipelineLayout()),
	    0
	 };

	 drawhandle.ds_handle = g_used_fullscreen_descriptors++;
	 assert(size_t(g_used_fullscreen_descriptors) <  g_fullscreen_descriptor_sets.size());
	 std::vector<VkDescriptorSet> set(1, {g_fullscreen_descriptor_sets[drawhandle.ds_handle]});
	 buffers::pbr::SetDescriptorAlbedo(renderpass::GetPassImageView("ui"), set);
	 buffers::pbr::SetDescriptorMetallicRoughness(renderpass::GetPassImageView("pbr"), set);
	 buffers::pbr::SetDescriptorNormal(renderpass::GetPassImageView("pbr"), set);
	 buffers::pbr::SetDescriptorAO(renderpass::GetPassImageView("pbr"), set);
	 
	 return drawhandle;
      }
   }
}    
