#include "gGlfwVulkan.h"
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <math.h>
#include <optional>
#include <functional>
#include <array>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../types/mat4x4f.h"
#include "vulkan_types.h"


using namespace fw;
struct vkVertex : public fw::Vertex
{
   static VkVertexInputBindingDescription GetBindingDescription()
   {
      VkVertexInputBindingDescription binding_description = {};
      binding_description.binding = 0;
      binding_description.stride = sizeof(fw::Vertex);
      binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      return binding_description;
   }
   static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions()
   {
      std::array<VkVertexInputAttributeDescription, 4> attribute_description = {};
      attribute_description[0].binding = 0;
      attribute_description[0].location = 0;
      attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_description[0].offset = offsetof(Vertex, position); // position

      attribute_description[1].binding = 0;
      attribute_description[1].location = 1;
      attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_description[1].offset = offsetof(Vertex, normal); // normal
      
      attribute_description[2].binding = 0;
      attribute_description[2].location = 2;
      attribute_description[2].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_description[2].offset = offsetof(Vertex, color); // colour
      
      attribute_description[3].binding = 0;
      attribute_description[3].location = 3;
      attribute_description[3].format = VK_FORMAT_R32G32_SFLOAT;
      attribute_description[3].offset = offsetof(Vertex, uv); // uvs

      return attribute_description;
   }
};

#include "pbr_pipeline.h"

namespace fwvulkan
{
   // todo: I should make a header of forward declarations
   namespace renderpass {
      bool CreateDefaultRenderPass(hash::string passname, VkExtent2D extent, VkFormat format, VkImageLayout layout);
   }
   // instance
   VkInstance g_instance;
   VkSurfaceKHR g_surface = VK_NULL_HANDLE;
   VkDebugUtilsMessengerEXT g_debug_messenger;
   extern GLFWwindow* g_window;
   extern bool g_resized;
   // device
   VkPhysicalDevice g_physical_device = VK_NULL_HANDLE;
   VkDevice g_logical_device = VK_NULL_HANDLE;
   VkQueue g_present_queue = VK_NULL_HANDLE;
   VkQueue g_graphics_queue = VK_NULL_HANDLE;
   // swapchain 
   VkSwapchainKHR g_swap_chain = VK_NULL_HANDLE;
   VkSurfaceFormatKHR g_swapchain_surface_format = {};
   // semaphores
   std::vector<VkSemaphore> g_image_available_semaphores;
   std::vector<VkSemaphore> g_render_finished_semaphores;
   std::vector<VkFence> g_in_flight_fences;
   const int g_max_frames_in_flight = 2;
   unsigned int g_current_frame = 0;
   unsigned int g_flight_frame = 0;
   /////
   // todo: this sucks
   VkDescriptorPool g_shared_descriptor_pool;
   std::vector<VkDescriptorSet> g_shared_descriptor_sets;
   

   
   DefaultUniforms ubo = {};
   VkDescriptorSetLayout g_shared_descriptor_set_layout;
   std::vector<VkBuffer> g_uniformBuffers;
   std::vector<VkDeviceMemory> g_uniformBuffersMemory;
   std::vector<void *> g_uniformBuffersMapped;

   // renderpass
   // the default one used for the swapchain
   struct DrawHandle;
   struct PassHandle
   {
      VkFramebuffer CurrentFrame()
      {
	 if(frame_buffers.size() == 0) return VK_NULL_HANDLE;
	 return frame_buffers[g_current_frame%frame_buffers.size()];
      }
      VkRenderPass pass = VK_NULL_HANDLE;
      VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
      VkExtent2D extent;
      VkFormat image_format;
      std::vector<VkImage> images;
      std::vector<VkImageView> image_views;
      std::vector<VkDeviceMemory> image_mems;
      std::vector<VkFramebuffer> frame_buffers;
      std::vector<DrawHandle> draws;
      std::vector<fw::hash::string> wait_passes;
   };
   PassHandle g_swapchain_pass;
   std::map<fw::hash::string, PassHandle> g_pass_map;
   struct SemaphoreHandle
   {
      std::vector<VkSemaphore> image_available;
      std::vector<VkSemaphore> render_finished;
      std::vector<VkFence> in_flight_fences;      
   };
   std::map<fw::hash::string, SemaphoreHandle> g_semaphore_map;
   // shaders
   typedef std::map<fw::hash::string, VkShaderModule> shader_map;
   std::array<shader_map, fw::shader::e_count> g_shaders;
   // pipeline
   struct PipelineHandle
   {
      VkPipeline pipeline = VK_NULL_HANDLE;
      VkPipeline depth_pipeline = VK_NULL_HANDLE;
      VkPipelineLayout layout = VK_NULL_HANDLE;
   };
   std::map<uint32_t, PipelineHandle> g_pipe_map;
   
   // commands
   VkCommandPool g_command_pool = VK_NULL_HANDLE;
   // std::vector<VkCommandBuffer> g_command_buffers;
   // vertex buffers

   std::map<uint32_t, VBHandle> g_vb_map;
   std::map<uint32_t, IBHandle> g_ib_map;
   std::map<uint32_t, IMHandle> g_im_map;         // readonly/sampled.
   std::map<fw::hash::string, IMHandle> g_rt_map; // render targets.
   std::map<uint32_t, SamHandle> g_sam_map;
   // mesh
   std::vector<Mesh*> g_meshes;
   std::map<fw::Mesh *, DrawHandle> g_drawhandles;
   //
   bool g_enable_validation_layers = false;
   const std::vector<const char*> g_instance_extensions = {
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
   };
   const std::vector<const char *> g_device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
      // VK_KHR_DYNAMIC_RENDERING_NAME,
      // VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,

   };
   // Note: These layers require you to run Vulkan/1.3.280.1/setup-env.sh prior to running the executable.
   const std::vector<const char *> g_validation_layers = {
      "VK_LAYER_KHRONOS_validation",
      "VK_LAYER_RENDERDOC_Capture",
      // "VK_LAYER_MESA_device_select",
      // "VK_LAYER_NV_optimus",
      // "VK_LAYER_LUNARG_api_dump",
      // "VK_LAYER_LUNARG_gfxreconstruct",
      // "VK_LAYER_KHRONOS_profiles",
      // "VK_LAYER_LUNARG_screenshot",
   };
   struct QueueFamilyIndices
   {
      //todo: do we really need std::optional? why?
      std::optional<uint32_t> graphics_family;
      std::optional<uint32_t> present_family;
      bool IsComplete()
      {
	 return graphics_family.has_value() && present_family.has_value();
      }
   };
   struct SwapChainSupportDetails
   {
      VkSurfaceCapabilitiesKHR capabilites;
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR> present_modes;
   };
   namespace utils
   {
      uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
      {
	 VkPhysicalDeviceMemoryProperties memory_properties;
	 vkGetPhysicalDeviceMemoryProperties(g_physical_device, &memory_properties);

	 for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
	 {
	    if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
	    {
	       return i;
	    }
	 }
	 throw std::runtime_error("failed to find suitable memory type!");
      }
      VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
      {
	 for (VkFormat format : candidates)
	 {
	    VkFormatProperties props;
	    vkGetPhysicalDeviceFormatProperties(g_physical_device, format, &props);

	    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
	    {
	       return format;
	    }
	    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
	    {
	       return format;
	    }
	 }
	 throw std::runtime_error("failed to find supported format!");
      }
      VkFormat FindDepthFormat() {
	 return FindSupportedFormat(
	    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	    );
      }
      bool hasStencilComponent(VkFormat format)
      {
	 return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
      }
      
      typedef std::function<void(VkCommandBuffer)> RecordCB;
      void RecordAndSubmit(RecordCB func)
      {
	 VkCommandBufferAllocateInfo allocInfo{};
	 allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	 allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	 allocInfo.commandPool = g_command_pool;
	 allocInfo.commandBufferCount = 1;

	 VkCommandBuffer cb;
	 vkAllocateCommandBuffers(g_logical_device, &allocInfo, &cb);

	 VkCommandBufferBeginInfo beginInfo{};
	 beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	 beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	 vkBeginCommandBuffer(cb, &beginInfo);
	 func(cb);
	 vkEndCommandBuffer(cb);

	 VkSubmitInfo submitInfo{};
	 submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	 submitInfo.commandBufferCount = 1;
	 submitInfo.pCommandBuffers = &cb;

	 if(vkQueueSubmit(g_graphics_queue, 1, &submitInfo, VK_NULL_HANDLE)  != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to submit one time command buffer!");
	 }
	 if(vkQueueWaitIdle(g_graphics_queue) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to wait for one time command buffer!");
	 }
	 log::debug("One time command buffer {} run, freeing.", size_t(cb));
	 vkFreeCommandBuffers(g_logical_device, g_command_pool, 1, &cb);
      }
      void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
      {
	 (void)format;
	 RecordAndSubmit([&](VkCommandBuffer cb)
	 {
	    log::debug("Transitioning image {} from {} to {}", size_t(image), oldLayout, newLayout);
	    VkImageMemoryBarrier barrier{};
	    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	    barrier.oldLayout = oldLayout;
	    barrier.newLayout = newLayout;

	    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	    barrier.image = image;
	    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	    barrier.subresourceRange.baseMipLevel = 0;
	    barrier.subresourceRange.levelCount = 1;
	    barrier.subresourceRange.baseArrayLayer = 0;
	    barrier.subresourceRange.layerCount = 1;

	    VkPipelineStageFlags source;
	    VkPipelineStageFlags dest;

	    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	    {
	       barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	       if (hasStencilComponent(format))
	       {
		  barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	       }
	    }
	    else
	    {
	       barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	    }

	    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	    {
	       barrier.srcAccessMask = 0;
	       barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	       source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	       dest = VK_PIPELINE_STAGE_TRANSFER_BIT;
	    }
	    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	    {
	       barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	       barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	       source = VK_PIPELINE_STAGE_TRANSFER_BIT;
	       dest = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	    }
	    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	    {
	       barrier.srcAccessMask = 0;
	       barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	       source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	       dest = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	    }
	    else
	    {
	       throw std::invalid_argument("unsupported layout transition!");
	    }
	    vkCmdPipelineBarrier(cb, source, dest, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	 });
      }
      void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	 RecordAndSubmit([&](VkCommandBuffer cb)
	 {
	    log::debug("Record CopyBufferToImage");
	    VkBufferImageCopy region{};
	    region.bufferOffset = 0;
	    region.bufferRowLength = 0;
	    region.bufferImageHeight = 0;

	    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	    region.imageSubresource.mipLevel = 0;
	    region.imageSubresource.baseArrayLayer = 0;
	    region.imageSubresource.layerCount = 1;

	    region.imageOffset = {0, 0, 0};
	    region.imageExtent = { width, height, 1 };
	    
	    vkCmdCopyBufferToImage(cb, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	 });
      }
   }
   namespace buffers
   {
      // todo: this has a bunch of duplicated code in it, make a generic create image.
      // buffers::internal::CreateImage
      void CreateImage(int width, int height, VkFormat format, VkImageUsageFlags usage, VkImage& image, VkDeviceMemory& image_memory)
      {
	 log::debug("Create Image");
	 assert(width > 0 && height > 0);
	 VkImageCreateInfo image_ci = {};
	 image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	 image_ci.imageType = VK_IMAGE_TYPE_2D;
	 image_ci.extent.width = width;
	 image_ci.extent.height = height;
	 image_ci.extent.depth = 1;
	 image_ci.mipLevels = 1;
	 image_ci.arrayLayers = 1;
	 image_ci.format = format;
	 image_ci.tiling = VK_IMAGE_TILING_OPTIMAL; // todo: maybe different?
	 image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	 image_ci.usage = usage;
	 image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
	 image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	 image_ci.flags = 0;
	 // todo: check on these.
	 image_ci.queueFamilyIndexCount = 0;
	 image_ci.pQueueFamilyIndices = nullptr;

	 if (vkCreateImage(g_logical_device, &image_ci, nullptr, &image) != VK_SUCCESS) {
	    throw std::runtime_error("failed to create image!");
	 }
	 
	 VkMemoryAllocateInfo alloc_info = {};
	 alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	    
	 VkMemoryRequirements mem_requirements;
	 vkGetImageMemoryRequirements(g_logical_device, image, &mem_requirements);
	    
	 alloc_info.allocationSize = mem_requirements.size;
	 alloc_info.memoryTypeIndex = utils::FindMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	 
	 if (vkAllocateMemory(g_logical_device, &alloc_info, nullptr, &image_memory) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to allocate image mem!");
	 }
	 if (vkBindImageMemory(g_logical_device, image, image_memory, 0) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to failed to bind image mem!");
	 }
	 log::debug("Created Image ({}, {}) requirements size: {} align: {} vkmem: {}", width, height, mem_requirements.size, mem_requirements.alignment, (size_t)image_memory);
      }
      VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT)
      {
	 VkImageViewCreateInfo image_view_ci = {};
	 image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	 image_view_ci.image = image;
	 image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	 image_view_ci.format = format;
	 image_view_ci.subresourceRange.aspectMask = aspect;
	 image_view_ci.subresourceRange.baseMipLevel = 0;
	 image_view_ci.subresourceRange.levelCount = 1;
	 image_view_ci.subresourceRange.baseArrayLayer = 0;
	 image_view_ci.subresourceRange.layerCount = 1;

	 image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	 image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	 image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	 image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	 VkImageView view;
	 if (vkCreateImageView(g_logical_device, &image_view_ci, nullptr, &view) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to create texture image view!");
	 }
	 return view;
      }
      VkDescriptorSetLayout CreateDescriptorSetLayout(VkDescriptorType* types, VkShaderStageFlags* stage_flags, int num)
      {
	 log::debug("CreateDescriptorSetLayout");
	 assert(num < 16);
	 std::array<VkDescriptorSetLayoutBinding, 16> bindings;
		      
	 for (int i = 0; i < num; i++)
	 {
	    bindings[i].binding = i;
	    bindings[i].descriptorCount = 1;
	    bindings[i].descriptorType = types[i];
	    bindings[i].pImmutableSamplers = nullptr;
	    bindings[i].stageFlags = stage_flags[i];
	    log::debug("Descriptor binding {}: t:{} s:{}", i, types[i], stage_flags[i]);
	 }

	 VkDescriptorSetLayoutCreateInfo layout_ci = {};
	 layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	 layout_ci.bindingCount = num;
	 layout_ci.pBindings = bindings.data();
	
	 VkDescriptorSetLayout descriptor_set;
	 if (vkCreateDescriptorSetLayout(g_logical_device, &layout_ci, nullptr, &descriptor_set) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to create descriptor set layout!");
	 }
	 return descriptor_set;
      }
      
      void CreateBuffer(void* source, size_t size, VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& memory)
      {
	 VkBufferCreateInfo buffer_ci = {};
	 buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	 buffer_ci.size = size;
	 buffer_ci.usage = usage;
	 buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	 if (vkCreateBuffer(g_logical_device, &buffer_ci, nullptr, &buffer) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to create buffer!");
	 }
	 VkMemoryRequirements requirements;
	 vkGetBufferMemoryRequirements(g_logical_device, buffer, &requirements);

	 VkMemoryAllocateInfo allocate_info = {};
	 allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	 allocate_info.allocationSize = requirements.size;
	 // todo: make sure we always want host visible and host coherent
	 auto property_bits = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	 allocate_info.memoryTypeIndex = utils::FindMemoryType(requirements.memoryTypeBits, property_bits);
	 if (vkAllocateMemory(g_logical_device, &allocate_info, nullptr, &memory) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to allocate buffer memory!");
	 }
	 vkBindBufferMemory(g_logical_device, buffer, memory, 0);
	 if(source != nullptr)
	 {
	    const char* buff_type = "unknown";
	    buff_type = usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT ? "copy" : buff_type;
	    buff_type = usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ? "vertex" : buff_type;
	    buff_type = usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT ? "index" : buff_type;
	    buff_type = usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT ? "uniform" : buff_type;
	    
	    void *data = nullptr; 
	    vkMapMemory(g_logical_device, memory, 0, buffer_ci.size, 0, &data);
	    log::debug("Copying to {} buffer {} from {}", buff_type, data, source);
	    memcpy(data, source, (size_t)buffer_ci.size);
	    vkUnmapMemory(g_logical_device, memory);
	 }
      }
      
      VkDescriptorPool CreateDescriptorPool()
      {
	 VkDescriptorPoolSize pool_sizes[2];
	 pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	 pool_sizes[0].descriptorCount = g_max_frames_in_flight;
	 pool_sizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	 pool_sizes[1].descriptorCount = g_max_frames_in_flight;

	 VkDescriptorPoolCreateInfo pool_ci{};
	 pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	 pool_ci.poolSizeCount = 2;
	 pool_ci.pPoolSizes = &pool_sizes[0];
	 pool_ci.maxSets = g_max_frames_in_flight;

	 VkDescriptorPool pool;
	 if (vkCreateDescriptorPool(g_logical_device, &pool_ci, nullptr, &pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
	 }
	 return pool;
      }
      
      // todo: make a handle for these
      void CreateUniformBuffer(VkDeviceSize buffer_size, VkBuffer& buffer, VkDeviceMemory& mem, void*& mapping)
      {
	 CreateBuffer(nullptr, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, buffer, mem);
	 vkMapMemory(g_logical_device, mem, 0, buffer_size, 0, &mapping);
      }
      
      void CreateDefaultUniformBuffers()
      {
	 VkDeviceSize buffer_size = sizeof(DefaultUniforms);
	 g_uniformBuffers.resize(g_max_frames_in_flight);
	 g_uniformBuffersMemory.resize(g_max_frames_in_flight);
	 g_uniformBuffersMapped.resize(g_max_frames_in_flight);

	 for (size_t i = 0; i < g_max_frames_in_flight; i++)
	 {
	    CreateUniformBuffer(buffer_size, g_uniformBuffers[i], g_uniformBuffersMemory[i], g_uniformBuffersMapped[i]);
	 }
      }

      void SetDescriptorUniformBuffer()
      {
	 log::debug("SetDescriptorUniformmBuffer");
	 for (size_t i = 0; i < g_shared_descriptor_sets.size(); i++)
	 {
            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = g_uniformBuffers[i];
            buffer_info.offset = 0;
            buffer_info.range = sizeof(DefaultUniforms);

            VkWriteDescriptorSet descriptorWrites[1] = {{}};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = g_shared_descriptor_sets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &buffer_info;

            vkUpdateDescriptorSets(g_logical_device, 1, descriptorWrites, 0, nullptr);
	 }
      }
      
      void SetDescriptorSampler(VkSampler sampler)
      {
	 log::debug("SetDescriptorSampler: {}", size_t(sampler));
	 for (size_t i = 0; i < g_shared_descriptor_sets.size(); i++)
	 {
	    VkDescriptorImageInfo sampler_info{};
	    sampler_info.sampler = sampler;

            VkWriteDescriptorSet descriptorWrites[1] = {{}};
	    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	    descriptorWrites[0].dstSet = g_shared_descriptor_sets[i];
	    descriptorWrites[0].dstBinding = 1;
	    descriptorWrites[0].dstArrayElement = 0;
	    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	    descriptorWrites[0].descriptorCount = 1;
	    descriptorWrites[0].pImageInfo = &sampler_info;
            vkUpdateDescriptorSets(g_logical_device, 1, descriptorWrites, 0, nullptr);
	 }
      }
      void SetDescriptorImage(VkImageView image_view, std::vector<VkDescriptorSet> image_sets, unsigned int dst_binding)
      {
	 log::debug("SetDescriptorImage: {}", size_t(image_view));
	 for (size_t i = 0; i < image_sets.size(); i++)
	 {
	    VkDescriptorImageInfo image_info{};
	    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	    image_info.imageView = image_view;

            VkWriteDescriptorSet descriptorWrites[1] = {{}};
	    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	    descriptorWrites[0].dstSet = image_sets[i];
	    descriptorWrites[0].dstBinding = dst_binding;
	    descriptorWrites[0].dstArrayElement = 0;
	    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	    descriptorWrites[0].descriptorCount = 1;
	    descriptorWrites[0].pImageInfo = &image_info;

            vkUpdateDescriptorSets(g_logical_device, 1, descriptorWrites, 0, nullptr);
	 }
      }

      // todo: this does more than set the image, it also sets the uniform buffer.
      // ----: That's not ideal, I think more correctly you would create descriptors and rebind them when recording a pass.
      // ----: so probably you need descriptor sets per draw handle, which can change the images and transforms.
      // ----: Global transforms/constants should probably be in a push constant, or bound to the pipeline differently.
      // ----: descriptor table or something.
      // todo: consider bindless descriptors: VK_EXT_descriptor_indexing
      void SetDescriptors(VkSampler sampler)
      {
	 log::debug("SetDescriptors: a: {} r: {}", "doot", "toot");
	 for (size_t i = 0; i < g_shared_descriptor_sets.size(); i++)
	 {
            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = g_uniformBuffers[i];
            buffer_info.offset = 0;
            buffer_info.range = sizeof(DefaultUniforms);

	    VkDescriptorImageInfo sampler_info{};
	    sampler_info.sampler = sampler;

            VkWriteDescriptorSet descriptorWrites[2] = {{}, {}};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = g_shared_descriptor_sets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &buffer_info;

	    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	    descriptorWrites[1].dstSet = g_shared_descriptor_sets[i];
	    descriptorWrites[1].dstBinding = 1;
	    descriptorWrites[1].dstArrayElement = 0;
	    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	    descriptorWrites[1].descriptorCount = 1;
	    descriptorWrites[1].pImageInfo = &sampler_info;
            vkUpdateDescriptorSets(g_logical_device, 2, descriptorWrites, 0, nullptr);
	 }
      }
      
      int CreateImageHandle(const unsigned int* image, size_t width, size_t height);
      int CreateSamplerHandle(VkFilter filtering, VkSamplerAddressMode uv_mode, bool enable_aniso);
      // these are bound to descriptor pool and descriptor_set_layout
      void CreateDescriptorSets()
      {
	 log::debug("CreateDescriptorSets");
	 std::vector<VkDescriptorSetLayout> layouts(g_max_frames_in_flight, g_shared_descriptor_set_layout);
	 VkDescriptorSetAllocateInfo alloc_info{};
	 alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	 alloc_info.descriptorPool = g_shared_descriptor_pool;
	 alloc_info.descriptorSetCount = g_max_frames_in_flight;
	 alloc_info.pSetLayouts = layouts.data();
	 g_shared_descriptor_sets.resize(g_max_frames_in_flight);
	 
	 if (vkAllocateDescriptorSets(g_logical_device, &alloc_info, g_shared_descriptor_sets.data()) != VK_SUCCESS)
	 {
            throw std::runtime_error("failed to allocate descriptor sets!");
	 }
	 auto sam_handle = CreateSamplerHandle(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, true);
	 buffers::SetDescriptors(g_sam_map[sam_handle].sampler);
      }
      VkCommandBuffer CreateCommandBuffer()
      {
	 VkCommandBufferAllocateInfo alloc_info = {};
	 alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	 alloc_info.commandPool = g_command_pool;
	 alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	 alloc_info.commandBufferCount = 1;
	    
	 VkCommandBuffer buffer;
	 if (vkAllocateCommandBuffers(g_logical_device, &alloc_info, &buffer) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to allocate command buffers!");
	 }
	 return buffer;
      }
      // todo: make this configurable, almost everything in here you would want to configure as a user.
      VkSampler CreateSampler(VkFilter filtering, VkSamplerAddressMode uv_mode, bool enable_aniso)
      {
	 VkPhysicalDeviceProperties properties{};
	 vkGetPhysicalDeviceProperties(g_physical_device, &properties);

	 VkSamplerMipmapMode mode;
	 switch(filtering)
	 {
	    case VK_FILTER_LINEAR: mode = VK_SAMPLER_MIPMAP_MODE_LINEAR; break;

	    case VK_FILTER_CUBIC_IMG: mode = VK_SAMPLER_MIPMAP_MODE_LINEAR; break;
	       // VK_FILTER_CUBIC_EXT is the same value as VK_FILTER_CUBIC_IMG
	       // case VK_FILTER_CUBIC_EXT: mode = VK_SAMPLER_MIPMAP_MODE_LINEAR; break;
	    case VK_FILTER_MAX_ENUM: mode = VK_SAMPLER_MIPMAP_MODE_MAX_ENUM; break;
	    case VK_FILTER_NEAREST: mode = VK_SAMPLER_MIPMAP_MODE_NEAREST; break;
	 }
	 if(mode == VK_SAMPLER_MIPMAP_MODE_NEAREST && enable_aniso) {log::debug("CreateSampler: aniso and nearest sampling might not be wanted");}
	 VkSamplerCreateInfo sampler_ci{};
	 sampler_ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	 sampler_ci.magFilter = filtering;
	 sampler_ci.minFilter = filtering;
	 sampler_ci.addressModeU = uv_mode;
	 sampler_ci.addressModeV = uv_mode;
	 sampler_ci.addressModeW = uv_mode;
	 sampler_ci.anisotropyEnable = enable_aniso ?  VK_FALSE : VK_TRUE;
	 sampler_ci.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	 sampler_ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	 sampler_ci.unnormalizedCoordinates = VK_FALSE;
	 sampler_ci.compareEnable = VK_FALSE;
	 sampler_ci.compareOp = VK_COMPARE_OP_ALWAYS;
	 sampler_ci.mipmapMode = mode;
	 VkSampler sampler = VK_NULL_HANDLE;
	 if (vkCreateSampler(g_logical_device, &sampler_ci, nullptr, &sampler) != VK_SUCCESS)
	 {
            throw std::runtime_error("failed to create texture sampler!");
	 }
	 return sampler;
      }
      int CreateSamplerHandle(VkFilter filtering, VkSamplerAddressMode uv_mode, bool enable_aniso)
      {
	 struct { VkFilter f; VkSamplerAddressMode u; bool a; } params = {filtering, uv_mode, enable_aniso};
	 auto hash = fw::hash::hash_buffer((const char*)&params, sizeof(params));
	 if (g_sam_map.find(hash) == g_sam_map.end())
	 {
	    g_sam_map[hash] = {CreateSampler(filtering, uv_mode, enable_aniso)};
	 }
	 return hash;
      }
      int CreateImageHandle(const unsigned int* image_buffer, size_t width, size_t height)
      {
	 if (image_buffer == nullptr) return 0;
	 // todo: grab type size, not 4.
	 size_t image_size = width*height*4;
	 uint32_t hash = hash::hash_buffer((const char*)image_buffer, image_size);
	 if (g_im_map.find(hash) == g_im_map.end())
	 {
	    VkBuffer copy_buffer = VK_NULL_HANDLE;
	    VkDeviceMemory copy_memory = VK_NULL_HANDLE;
	    {
	       // this shows the correct data
	       // log::debug("before image size: w:{} h:{} size:{}", width, height, image_size);
	       // for(int i = 0; i < height; i++)
	       // {
	       // 	  for(int ii = 0; ii < width; ii++)
	       // 	  {
	       // 	     log::debug_inline("{0:x},", image_buffer[i*width+ii]);
	       // 	  }
	       // 	  log::debug("");
	       // }
	       VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	       CreateBuffer((void*)image_buffer, image_size, usage, copy_buffer, copy_memory);
	       // void* data; // this shows the correct data
	       // log::debug("after image size: w:{} h:{} size:{}", width, height, image_size);
	       // vkMapMemory(g_logical_device, copy_memory, 0, width*height*4, 0, &data);
	       // for(int i = 0; i < height; i++)
	       // {
	       // 	  for(int ii = 0; ii < width; ii++)
	       // 	  {
	       // 	     log::debug_inline("{0:x},", ((unsigned int*)data)[i*width+ii]);
	       // 	  }
	       // 	  log::debug("");
	       // }
	       // vkUnmapMemory(g_logical_device, copy_memory);
	    }
	    VkImage image = VK_NULL_HANDLE;
	    VkDeviceMemory image_memory = VK_NULL_HANDLE;
	    {
	       const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	       CreateImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, usage, image, image_memory);
	    }
	    utils::TransitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	    utils::CopyBufferToImage(copy_buffer, image, width, height);
	    utils::TransitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	    
	    vkDestroyBuffer(g_logical_device, copy_buffer, nullptr);
	    vkFreeMemory(g_logical_device, copy_memory, nullptr);

	    VkImageView view = CreateImageView(image, VK_FORMAT_R8G8B8A8_SRGB);
	    
	    g_im_map[hash] = {image, view, image_memory, width, height};
	    log::debug("Created IMHandle: {}", hash);
	 }
	 else
	 {
	    log::debug("Reusing IMHandle: {}", hash);
	 }
	 return hash;
      }
      int CreateVertexBufferHandle(const fw::Vertex* vertices, int num_vertices)
      {
	 // uint32_t hash = hash::i32((const char*)vertices, num_vertices*sizeof(fw::Vertex));
	 uint32_t hash = hash::hash_buffer((const char*)vertices, num_vertices*sizeof(fw::Vertex));
	 if (g_vb_map.find(hash) == g_vb_map.end())
	 {
	    VkBuffer vertex_buffer;
	    VkDeviceMemory vertex_buffer_memory;
	    CreateBuffer((void*)vertices, sizeof(fw::Vertex) * num_vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_buffer, vertex_buffer_memory);
	    g_vb_map[hash] = {vertex_buffer, vertex_buffer_memory, (size_t)num_vertices};
	    log::debug("Created VBHandle: {}, verts: {}, size: {}", hash, num_vertices, sizeof(fw::Vertex) * num_vertices);
	 }
	 else
	 {
	    log::debug("Reusing VBHandle: {}", hash);
	 }
	 return hash;
      }
      int CreateIndexBufferHandle(const uint16_t* indices, int num_indices)
      {
	 uint32_t hash = hash::hash_buffer((const char*)indices, num_indices*sizeof(uint16_t));
	 if (g_ib_map.find(hash) == g_ib_map.end())
	 {
	    VkBuffer index_buffer;
	    VkDeviceMemory index_buffer_memory;
	    CreateBuffer((void*)indices, sizeof(uint16_t) * num_indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_buffer, index_buffer_memory);
	    g_ib_map[hash] = {index_buffer, index_buffer_memory, (size_t)num_indices};
	    log::debug("Created IBHandle: {}", hash);
	 }
	 else
	 {
	    log::debug("Reusing IBHandle: {}", hash);
	 }
	 return hash;
      }
   }

   namespace instance
   {
      VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
					    const VkAllocationCallbacks *pAllocator,
					    VkDebugUtilsMessengerEXT *pDebugMessenger)
      {
	 auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	 if (func != nullptr)
	 {
	    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	 }
	 else
	 {
	    return VK_ERROR_EXTENSION_NOT_PRESENT;
	 }
      }
      void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
					 const VkAllocationCallbacks *pAllocator)
      {
	 auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	 if (func != nullptr)
	 {
	    func(instance, debugMessenger, pAllocator);
	 }
      }


      static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
							  VkDebugUtilsMessageTypeFlagsEXT messagetype,
							  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
							  void* /*pUserData*/)
      {
	 switch(messagetype)
	 {
	    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
	       log::error("error: {}", pCallbackData->pMessage);
	       break;
	    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
	       log::warn("warn: {}", pCallbackData->pMessage);
	       break;
	    default:
	       log::debug("info: {}", pCallbackData->pMessage);
	 }

	 return VK_FALSE;
      }

      void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
      {
	 createInfo = {};
	 createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	 createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
	    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
	    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	 createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
	    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
	    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	 createInfo.pfnUserCallback = DebugCallback;
      }
      
      void SetupDebugMessenger()
      {
	 log::debug("Setup Debug Messenger");
	 VkDebugUtilsMessengerCreateInfoEXT create_info = {};
	 PopulateDebugMessengerCreateInfo(create_info);

	 if (CreateDebugUtilsMessengerEXT(g_instance, &create_info, nullptr, &g_debug_messenger) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to setup debug messenger!");
	 }
      }

      bool CheckValidationLayerSupport()
      {
	 uint32_t layer_count = 0;
	 uint32_t layers_found = 0;
	 vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	 std::vector<VkLayerProperties> available_layers(layer_count);
	 vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
	 for (const char *layer_name : g_validation_layers)
	 {
	    log::debug("finding: {}", layer_name);
	    bool layer_found = false;
	    for (const auto &layer_properties : available_layers)
	    {
	       log::debug("\tcomparing: {}", layer_properties.layerName);

	       if (strcmp(layer_name, layer_properties.layerName) == 0)
	       {
		  layer_found = true;
		  layers_found++;
		  break;
	       }
	    }
	    if (layer_found == false)
	    {
	       log::error("missing validation layer: '{}'", layer_name);
	    }
	 }
	 return layers_found == g_validation_layers.size();
      }

      void CreateInstance()
      {
	 log::debug("CreateInstance");
	 if (g_enable_validation_layers && !CheckValidationLayerSupport())
	 {
	    log::fatal("fatal, unable to enable valiadation layers, you may need to run setup-env.sh");
	 }
	 // app info
	 VkApplicationInfo app_info = {};
	 app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	 app_info.pApplicationName = "FrameworkApp";
	 app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	 app_info.pEngineName = "Framework";
	 app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	 app_info.apiVersion = VK_API_VERSION_1_3;
	 // create info
	 VkInstanceCreateInfo create_info = {};
	 create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	 create_info.pApplicationInfo = &app_info;

	 VkDebugUtilsMessengerCreateInfoEXT create_debug_info = {};
	 if (g_enable_validation_layers)
	 {
	    create_info.enabledLayerCount = static_cast<uint32_t>(g_validation_layers.size());
	    create_info.ppEnabledLayerNames = g_validation_layers.data();
	    PopulateDebugMessengerCreateInfo(create_debug_info);
	    create_info.pNext = &create_debug_info;
	 }
	 else
	 {
	    create_info.enabledLayerCount = 0;
	    create_info.pNext = nullptr;
	 }
	 //
	 uint32_t glfwExtensionCount = 0;
	 const char **glfwExtensions;
	 glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	 std::vector<const char *> required_extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	 if (g_enable_validation_layers)
	 {
	    required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	    // required_extensions.push_back("VK_EXT_debug_marker");
	    // required_extensions.push_back("VK_EXT_tooling_info");
	 }
	 for(auto ext : g_instance_extensions)
	 {
	    required_extensions.push_back(ext);
	 }
	 log::debug("required exts: ");
	 for(const char* ext : required_extensions)
	 {
	    log::debug("\t{}", ext);
	 }
	 create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
	 create_info.ppEnabledExtensionNames = required_extensions.data();
	 create_info.enabledLayerCount = 0;
	 uint32_t extensionCount = 0;
	 vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	 std::vector<VkExtensionProperties> extensions(extensionCount);
	 vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	 log::debug("available instance extensions:");
	 for (const auto &extension : extensions)
	 {
	    log::debug("\t{}", extension.extensionName);
	 }
	 // todo: VK_KHR_wayland_surface is supported, consider using that.
	 // ----: apparently renderdoc doesn't support it.
	 VkResult result = vkCreateInstance(&create_info, nullptr, &g_instance);
	 if (result != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to create vk instance!");
	 }
      }
      void CreateSurface()
      {
	 log::debug("Create Surface");
	 if (glfwCreateWindowSurface(g_instance, g_window, nullptr, &g_surface) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to create window surface!");
	 }
      }
   }
   // Device handling
   namespace device
   {
      QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
      {
	 QueueFamilyIndices indices;
	 uint32_t queue_family_count = 0;
	 vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
	 std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	 vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

	 int i = 0;
	 for (const auto &queue_family : queue_families)
	 {
	    if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
	    {
	       indices.graphics_family = i;
	    }

	    VkBool32 present_support = false;
	    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
	    if (present_support)
	    {
	       indices.present_family = i;
	    }

	    if (indices.IsComplete())
	    {
	       break;
	    }
	    i++;
	 }
	 return indices;
      }
      bool CheckDeviceExtensionSupport(VkPhysicalDevice physical_device, const std::vector<const char *> &device_extensions)
      {
	 uint32_t extension_count = 0;
	 vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
	 std::vector<VkExtensionProperties> extensions(extension_count);
	 vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extensions.data());
	 std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
	 log::debug("device extensions: ");
	 for (const auto &extension : extensions)
	 {
	    log::debug("\t{}", extension.extensionName);
	    required_extensions.erase(extension.extensionName);
	 }

	 return required_extensions.empty();
      }
      SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
      {
	 SwapChainSupportDetails details;
	 vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilites);
	 uint32_t format_count = 0;
	 vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

	 if (format_count != 0)
	 {
	    details.formats.resize(format_count);
	    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, details.formats.data());
	 }
	 uint32_t present_mode_count = 0;
	 vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
	 if (present_mode_count)
	 {
	    details.present_modes.resize(present_mode_count);
	    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count,
						      details.present_modes.data());
	 }
	 return details;
      }
   
      bool IsDeviceSuitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface,
			    const std::vector<const char *> &device_extensions)
      {
	 VkPhysicalDeviceProperties device_properties;
	 vkGetPhysicalDeviceProperties(physical_device, &device_properties);

	 VkPhysicalDeviceFeatures device_features;
	 vkGetPhysicalDeviceFeatures(physical_device, &device_features);

	 QueueFamilyIndices indices = FindQueueFamilies(physical_device, surface);
	 bool extensions_supported = CheckDeviceExtensionSupport(physical_device, device_extensions);
	 bool suitable_device = indices.IsComplete();
	 suitable_device = suitable_device && (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
					       device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
	 suitable_device = suitable_device && device_features.geometryShader;
	 suitable_device = suitable_device && device_features.samplerAnisotropy;
	 suitable_device = suitable_device && extensions_supported;

	 bool swap_chain_adequate = false;
	 if (extensions_supported)
	 {
	    SwapChainSupportDetails swap_chain_support = QuerySwapChainSupport(physical_device, surface);
	    swap_chain_adequate = !swap_chain_support.formats.empty();
	    swap_chain_adequate = swap_chain_adequate && !swap_chain_support.present_modes.empty();
	 }

	 if (!suitable_device)
	 {
	    log::debug("\t{} failed suitability test.", device_properties.deviceName);
	 }
	 return suitable_device;
      }
      void PrintPhysicalDeviceInfo()
      {
	 log::debug("Device info: ");
	 VkPhysicalDeviceProperties device_properties;
	 vkGetPhysicalDeviceProperties(g_physical_device, &device_properties);
	 log::debug("\tname: {}", device_properties.deviceName);
	 VkPhysicalDeviceLimits& limits = device_properties.limits;
	 log::debug("\tmax single alloc: {}", limits.maxMemoryAllocationCount); // this is the max single allocation size
	 VkPhysicalDeviceMemoryBudgetPropertiesEXT budget = {};
	 VkPhysicalDeviceMemoryProperties2 memory_properties = {};
	 memory_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
	 memory_properties.pNext = &budget;
	 vkGetPhysicalDeviceMemoryProperties2(g_physical_device, &memory_properties);
	 VkPhysicalDeviceMemoryProperties& memory_physical = memory_properties.memoryProperties;
	 for(uint32_t i = 0; i < memory_physical.memoryTypeCount; i++)
	 {
	    log::debug("\tmemtype index: {}", memory_physical.memoryTypes[i].propertyFlags);
	    log::debug("\tmemtype flags: {}", memory_physical.memoryTypes[i].heapIndex);
	 }
	 for(uint32_t i = 0; i < memory_physical.memoryHeapCount; i++)
	 {
	    log::debug("\tmemheap size: {}", memory_physical.memoryHeaps[i].size);
	    log::debug("\tmemheap flags: {}", memory_physical.memoryHeaps[i].flags);
	    // if (budget) // this require VK_EXT_memory_budget to be enabled
	    {
	       log::debug("\tmemheap budget: {}", budget.heapBudget[i]);
	       log::debug("\tmemheap usage: {}", budget.heapUsage[i]);
	    }
	 }
	 
      }
      void PickPhysicalDevice()
      {
	 log::debug("Pick Physical Device");
	 uint32_t device_count = 0;
	 vkEnumeratePhysicalDevices(g_instance, &device_count, nullptr);
	 if (device_count == 0)
	 {
	    throw std::runtime_error("failed to find GPUs with Vulkan supported!!");
	 }
	 std::vector<VkPhysicalDevice> devices(device_count);
	 VkPhysicalDeviceProperties device_properties;

	 vkEnumeratePhysicalDevices(g_instance, &device_count, devices.data());
	 log::debug("{} devices found:", device_count);
	 for (const auto &device : devices)
	 {
	    vkGetPhysicalDeviceProperties(device, &device_properties);
	    log::debug("\t{} .", device_properties.deviceName);
	 }
	 for (const auto &device : devices)
	 {
	    if (IsDeviceSuitable(device, g_surface, g_device_extensions))
	    {
	       g_physical_device = device;
	       break;
	    }
	 }
	 if (g_physical_device == VK_NULL_HANDLE)
	 {
	    std::runtime_error("Failed to find suitable a GPU!");
	 }
	 else
	 {
	    log::debug("chosen as physical device");
	 }
      }
      void CreateLogicalDevice()
      {
	 log::debug("Create Logical Device");
	 QueueFamilyIndices indices = FindQueueFamilies(g_physical_device, g_surface);

	 std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	 std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(), indices.present_family.value()};
	 float queue_priority = 1.0f;
	 for (uint32_t queue_family : unique_queue_families)
	 {
	    VkDeviceQueueCreateInfo queue_create_info = {};
	    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	    queue_create_info.queueFamilyIndex = queue_family;
	    queue_create_info.queueCount = 1;

	    queue_create_info.pQueuePriorities = &queue_priority;
	    queue_create_infos.push_back(queue_create_info);
	 }
	 VkPhysicalDeviceFeatures device_features = {};
	 device_features.samplerAnisotropy = VK_TRUE;
	 
	 VkDeviceCreateInfo create_info = {};
	 create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	 create_info.pQueueCreateInfos = queue_create_infos.data();
	 create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	 create_info.pEnabledFeatures = &device_features;
	 create_info.enabledExtensionCount = static_cast<uint32_t>(g_device_extensions.size());
	 create_info.ppEnabledExtensionNames = g_device_extensions.data();
	 create_info.enabledLayerCount = 0;
#if 0
         // todo: passing validation layers here is deprecated I think.
	 // but worth seeing if it's just done differently.
	 if (g_enable_validation_layers)
	 {
	    create_info.enabledLayerCount = static_cast<uint32_t>(g_validation_layers.size());
	    create_info.ppEnabledLayerNames = g_validation_layers.data();
	 }
#endif 
	 if (vkCreateDevice(g_physical_device, &create_info, nullptr, &g_logical_device) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to create logical device!");
	 }

	 vkGetDeviceQueue(g_logical_device, indices.graphics_family.value(), 0, &g_graphics_queue);
	 vkGetDeviceQueue(g_logical_device, indices.present_family.value(), 0, &g_present_queue);
	 PrintPhysicalDeviceInfo();
      }

   }
   namespace swapchain
   {
      void CreateCommandPool()
      {
	 log::debug("Create Command Pool");
	 assert(g_command_pool == VK_NULL_HANDLE);
	 QueueFamilyIndices queue_family_indices = device::FindQueueFamilies(g_physical_device, g_surface);
	 VkCommandPoolCreateInfo pool_create_info = {};
	 pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	 pool_create_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
	 pool_create_info.flags = 0;

	 if (vkCreateCommandPool(g_logical_device, &pool_create_info, nullptr, &g_command_pool) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to create command pool!");
	 }
      }
      void CreateSwapchainFrameBuffers()
      {
	 log::debug("CreateSwapchainFrameBuffers");
	 auto& pass = g_pass_map["swapchain"];
	 assert(pass.frame_buffers.size() == 0);
	 pass.frame_buffers.resize(pass.image_views.size());
	 for (size_t i = 0; i < pass.image_views.size(); i++)
	 {
	    // todo: might need to add roughness here.
	    // No. Maybe a Gbuffer. The rougness of a model is _not_ going to mutate between passes.
	    // and it also will not need to be written as an attachement, we just record binds.
	    std::array<VkImageView,2> attachments = {pass.image_views[i], g_rt_map["depth"].view};
	    VkFramebufferCreateInfo framebuffer_create_info = {};
	    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	    framebuffer_create_info.renderPass = pass.pass;
	    framebuffer_create_info.attachmentCount = attachments.size();
	    framebuffer_create_info.pAttachments = attachments.data();
	    framebuffer_create_info.width = pass.extent.width;
	    framebuffer_create_info.height = pass.extent.height;
	    framebuffer_create_info.layers = 1;

	    if (vkCreateFramebuffer(g_logical_device, &framebuffer_create_info, nullptr, &pass.frame_buffers[i]) !=
		VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create framebuffer!");
	    }
	 }
      }
      void CleanupSwapChain()
      {
	 log::debug("CleanupSwapChain");
	 assert(g_swap_chain != VK_NULL_HANDLE);
	 // todo: not all passes are going to want to be cleaned up when the swapchain is.
	 for (auto pass : g_pass_map)
	 {
	    for (auto framebuffer : pass.second.frame_buffers)
	    {
	       vkDestroyFramebuffer(g_logical_device, framebuffer, nullptr);
	    }
	    for (auto view : pass.second.image_views)
	    {
	       vkDestroyImageView(g_logical_device, view, nullptr);
	    }
	    if (pass.first != hash::string("swapchain"))
	    {
	       // note: we don't allocate these for the swapchain
	       for (auto image : pass.second.images)
	       {
		  vkDestroyImage(g_logical_device, image, nullptr);
	       }
	    }
	    for (auto mem : pass.second.image_mems)
	    {
	       vkFreeMemory(g_logical_device, mem, nullptr);
	    }
	    pass.second.frame_buffers.clear();
	    pass.second.image_views.clear();
	    pass.second.images.clear();
	    pass.second.image_mems.clear();
	    //
	    vkFreeCommandBuffers(g_logical_device, g_command_pool, 1, &pass.second.cmd_buffer);
	    vkDestroyRenderPass(g_logical_device, pass.second.pass, nullptr);
	 }
	 vkDestroySwapchainKHR(g_logical_device, g_swap_chain, nullptr);
	 
#if 0 // todo: use this kind of thing to replace g_semaphore_map
	 log::debug("cleaning up semaphores");
	 for (auto pass : g_pass_map)
	 {
	    for (int i = 0; i < fwvulkan::g_max_frames_in_flight; i++)
	    {
	       log::debug("cleaning up {}:{} ({})", i, fwvulkan::g_current_frame%fwvulkan::g_max_frames_in_flight, fwvulkan::g_current_frame);

	       if(0)
	       {
		  // this requires VK_SEMAPHORE_TYPE_TIMELINE created semaphores
		  const VkSemaphore semas[] = {pass.second.render_finished[i]};
		  const uint64_t vals[] = {1};
		  VkSemaphoreWaitInfo waitinfo;
		  waitinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		  waitinfo.flags = VK_SEMAPHORE_WAIT_ANY_BIT;
		  waitinfo.pSemaphores = semas;
		  waitinfo.pValues = vals;
		  vkWaitSemaphores(g_logical_device, &waitinfo, 1000000);
		  log::debug("done waiting");
	       }
	       vkDestroySemaphore(g_logical_device, pass.second.image_available[i], nullptr);
	       vkDestroySemaphore(g_logical_device, pass.second.render_finished[i], nullptr);
	       vkDestroyFence(g_logical_device, pass.second.in_flight_fences[i], nullptr);
	    }
	    log::debug("done");
	 }
#endif
	 auto& rt = g_rt_map["depth"];
	 vkDestroyImage(g_logical_device, rt.image, nullptr);
	 vkDestroyImageView(g_logical_device, rt.view, nullptr);
	 vkFreeMemory(g_logical_device, rt.image_mem, nullptr);
	 g_rt_map.erase("depth");
	 
	 g_pass_map.clear();
	 g_swap_chain = VK_NULL_HANDLE;
      }
      void CreateSwapchainImageViews()
      {
	 log::debug("Create Swapchain Image Views");
	 auto& pass = g_pass_map["swapchain"];
	 assert(pass.image_views.size() == 0);
	 pass.image_views.resize(pass.images.size());

	 for (size_t i = 0; i < pass.images.size(); i++)
	 {
	    pass.image_views[i] = buffers::CreateImageView(pass.images[i], pass.image_format);
	 }
      }
      VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> available_formats)
      {
	 for (const auto &available_format : available_formats)
	 {
	    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
		available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
	    {
	       return available_format;
	    }
	 }
	 return available_formats[0];
      }
      VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &available_present_modes)
      {
	 VkPresentModeKHR best_mode = VK_PRESENT_MODE_FIFO_KHR;
	 for (const auto &available_present_mode : available_present_modes)
	 {
	    if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
	    {
	       return available_present_mode;
	    }
	    else if (available_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
	    {
	       best_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	    }
	 }
	 return best_mode;
      }
      // todo: this should be called once and stored on swapchain create / recreate.
      // ----: right now it's awkwardly stored on the swapchain renderpass, which is just wrong.
      // ----: the swap chain is unique, and should be treated as such, rather than stored as a render pass.
      VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window)
      {
	 if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	 {
	    return capabilities.currentExtent;
	 }
	 else
	 {
	    int width, height;
	    glfwGetFramebufferSize(window, &width, &height);

	    VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
	    actual_extent.width = std::max(capabilities.minImageExtent.width,
					   std::min(capabilities.maxImageExtent.width, actual_extent.width));
	    actual_extent.height = std::max(capabilities.minImageExtent.height,
					    std::min(capabilities.maxImageExtent.height, actual_extent.height));

	    return actual_extent;
	 }
      }
      void CreateSwapChain()
      {
	 log::debug("Create Swap Chain");
	 assert(g_swap_chain == VK_NULL_HANDLE);

	 SwapChainSupportDetails swap_chain_support = device::QuerySwapChainSupport(g_physical_device, g_surface);
	 VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(swap_chain_support.formats);
	 VkPresentModeKHR present_mode = ChooseSwapPresentMode(swap_chain_support.present_modes);
	 VkExtent2D extent = ChooseSwapExtent(swap_chain_support.capabilites, g_window);
	 uint32_t image_count = swap_chain_support.capabilites.minImageCount + 1;

	 if (swap_chain_support.capabilites.maxImageCount > 0 && image_count > swap_chain_support.capabilites.maxImageCount)
	 {
	    image_count = swap_chain_support.capabilites.maxImageCount;
	 }

	 VkSwapchainCreateInfoKHR create_info = {};
	 create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	 create_info.surface = g_surface;
	 create_info.minImageCount = image_count;
	 create_info.imageFormat = surface_format.format;
	 create_info.imageColorSpace = surface_format.colorSpace;
	 create_info.imageExtent = extent;
	 create_info.imageArrayLayers = 1;
	 create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	 QueueFamilyIndices indices = device::FindQueueFamilies(g_physical_device, g_surface);
	 uint32_t queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};

	 if (indices.graphics_family != indices.present_family)
	 {
	    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	    create_info.queueFamilyIndexCount = 2;
	    create_info.pQueueFamilyIndices = queue_family_indices;
	 }
	 else
	 {
	    log::debug("graphics_family is present_family");
	    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	    create_info.queueFamilyIndexCount = 0;
	    create_info.pQueueFamilyIndices = nullptr;
	 }
	 create_info.preTransform = swap_chain_support.capabilites.currentTransform;
	 create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	 create_info.presentMode = present_mode;
	 create_info.clipped = VK_TRUE;
	 create_info.oldSwapchain = VK_NULL_HANDLE;

	 if (vkCreateSwapchainKHR(g_logical_device, &create_info, nullptr, &g_swap_chain) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to create swap chain!");
	 }

	 renderpass::CreateDefaultRenderPass("swapchain", extent, surface_format.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	 auto& pass = g_pass_map["swapchain"];
	 vkGetSwapchainImagesKHR(g_logical_device, g_swap_chain, &image_count, nullptr);
	 pass.images.resize(image_count);
	 vkGetSwapchainImagesKHR(g_logical_device, g_swap_chain, &image_count, pass.images.data());
      }
   }
   namespace renderpass
   {
      void CreatePassImages(fw::hash::string passname)
      {
	 auto& pass = g_pass_map[passname];

	 if(pass.extent.width == 0 || pass.extent.width == 0)
	 {
	    pass.extent = g_pass_map["swapchain"].extent;
	 }
	 log::debug("CreatePassImages '{}' ({}, {}) format: {}", passname.m_literal,pass.extent.width, pass.extent.height, pass.image_format);

	 assert(pass.images.size() == 0);
	 assert(pass.frame_buffers.size() == 0);
	 assert(pass.image_views.size() == 0);
	 assert(pass.image_mems.size() == 0);
	 
	 pass.images.resize(g_max_frames_in_flight);
	 pass.image_views.resize(g_max_frames_in_flight);
	 pass.image_mems.resize(g_max_frames_in_flight);
         pass.frame_buffers.resize(g_max_frames_in_flight);
	 
         // back buffer format
	 const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	 
	 for (size_t i = 0; i < pass.images.size(); i++)
	 {
	    buffers::CreateImage(pass.extent.width, pass.extent.height, pass.image_format, usage, pass.images[i], pass.image_mems[i]);
	    pass.image_views[i] = buffers::CreateImageView(pass.images[i], pass.image_format);
	    // todo:  may need to add roughness.
	    // Once again, no. for same reason as before, roughness belongs to models.
	    VkImageView attachments[] = { pass.image_views[i] };
	    VkFramebufferCreateInfo framebuffer_create_info = {};
	    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	    framebuffer_create_info.renderPass = pass.pass;
	    framebuffer_create_info.attachmentCount = 1;
	    framebuffer_create_info.pAttachments = attachments;
	    framebuffer_create_info.width = pass.extent.width;
	    framebuffer_create_info.height = pass.extent.height;
	    framebuffer_create_info.layers = 1;

	    if (vkCreateFramebuffer(g_logical_device, &framebuffer_create_info, nullptr, &pass.frame_buffers[i]) !=
		VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create framebuffer!");
	    }
	 }
      }

      VkAttachmentDescription DefaultColourAttachment()
      {
	 VkAttachmentDescription colour_attachment = {};
	 colour_attachment.format = VK_FORMAT_R8G8B8A8_SRGB;
	 colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	 colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	 colour_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	 colour_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	 colour_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	 colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	 colour_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	 return colour_attachment;
      }
      VkAttachmentDescription DefaultDepthAttachment()
      {
	 VkAttachmentDescription depth_attachment = {};
	 depth_attachment.format = utils::FindDepthFormat();
	 depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	 depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	 depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	 depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	 depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	 depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	 depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	 return depth_attachment;
      }
      bool CreateDefaultRenderPass(hash::string passname, VkExtent2D extent, VkFormat format, VkImageLayout layout)
      {
	 if(g_pass_map.find(passname) == g_pass_map.end())
	 {
	    log::debug("Creating Default Renderpass: '{}'", passname.m_literal);
	    
	    auto colour_attachment = DefaultColourAttachment();
	    auto depth_attachment = DefaultDepthAttachment();
	    
	    colour_attachment.finalLayout = layout;

	    VkAttachmentReference color_attachment_reference = {};
	    color_attachment_reference.attachment = 0;
	    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	    VkAttachmentReference depth_attachment_reference = {};
	    depth_attachment_reference.attachment = 1;
	    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	    // todo: may need roughness... not liking that I need to make a pipeline.
	    // ....: ree. Nope, see above.
	    // It does need a slot for binding however, which is in our descriptor set layout.
	    VkSubpassDescription subpass_description = {};
	    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	    subpass_description.colorAttachmentCount = 1;
	    subpass_description.pColorAttachments = &color_attachment_reference;
	    subpass_description.pDepthStencilAttachment = &depth_attachment_reference;

	    VkPipelineStageFlags stages = {};
	    stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	    stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

	    VkAccessFlags access = {};
	    access |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	    access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	    
	    VkSubpassDependency subpass_dependency = {};
	    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	    subpass_dependency.dstSubpass = 0;
	    subpass_dependency.srcStageMask = stages;
	    subpass_dependency.srcAccessMask = 0;
	    subpass_dependency.dstStageMask = stages;
	    subpass_dependency.dstAccessMask = access;

	    std::array<VkAttachmentDescription, 2> attachments = {colour_attachment, depth_attachment};
	    VkRenderPassCreateInfo render_pass_ci = {};
	    render_pass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	    render_pass_ci.attachmentCount = attachments.size();
	    render_pass_ci.pAttachments = attachments.data();
	    render_pass_ci.subpassCount = 1;
	    render_pass_ci.pSubpasses = &subpass_description;
	    render_pass_ci.dependencyCount = 1;
	    render_pass_ci.pDependencies = &subpass_dependency;
	    
	    VkRenderPass pass;
	    if (vkCreateRenderPass(g_logical_device, &render_pass_ci, nullptr, &pass) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create render pass!");
	    }
	    VkCommandBuffer buffer = buffers::CreateCommandBuffer();
	    g_pass_map[passname] = {pass, buffer, extent, format, {}, {}, {}, {}, {}, {}};
	    return true;
	 }
	 return false;
      }
      
      void CreateDepthBuffer()
      {
	 VkFormat format = utils::FindDepthFormat();
	 VkImage depth_image; VkDeviceMemory depth_memory;
	 // todo: more suckage.
	 auto extent = g_pass_map["swapchain"].extent;
	 buffers::CreateImage(extent.width, extent.height, format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depth_image, depth_memory);
	 auto depth_view = buffers::CreateImageView(depth_image, format, VK_IMAGE_ASPECT_DEPTH_BIT);
	 g_rt_map["depth"] = { depth_image, depth_view, depth_memory, extent.width, extent.height };
	 utils::TransitionImageLayout(depth_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
      }
      
   }
   namespace barriers
   {
      bool CreatePassSemaphores(hash::string);
   }
   namespace swapchain
   {
      void RecreateSwapChain()
      {
	 // todo: the namespace split for this single function is awkward, workout better dependencies / declaration order?
	 // todo: CreateGraphicsPipeline(); needs to become invalidate pipeline states or something similar.
	 log::debug("Recreate SwapChain");
	 int width = 0, height = 0;
	 glfwGetFramebufferSize(g_window, &width, &height);
	 while (width == 0 || height == 0)
	 {
	    glfwGetFramebufferSize(g_window, &width, &height);
	    glfwWaitEvents();
	 }
	 vkDeviceWaitIdle(g_logical_device);
	 CleanupSwapChain();
	 CreateSwapChain();
	 renderpass::CreateDepthBuffer();
	 CreateSwapchainImageViews();
	 CreateSwapchainFrameBuffers();
      }
   }
   namespace shaders
   {
//       typedef enum VkShaderStageFlagBits {
//     VK_SHADER_STAGE_VERTEX_BIT = 0x00000001,
//     VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT = 0x00000002,
//     VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 0x00000004,
//     VK_SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
//     VK_SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
//     VK_SHADER_STAGE_COMPUTE_BIT = 0x00000020,
//     VK_SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
//     VK_SHADER_STAGE_ALL = 0x7FFFFFFF,
//     VK_SHADER_STAGE_RAYGEN_BIT_KHR = 0x00000100,
//     VK_SHADER_STAGE_ANY_HIT_BIT_KHR = 0x00000200,
//     VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR = 0x00000400,
//     VK_SHADER_STAGE_MISS_BIT_KHR = 0x00000800,
//     VK_SHADER_STAGE_INTERSECTION_BIT_KHR = 0x00001000,
//     VK_SHADER_STAGE_CALLABLE_BIT_KHR = 0x00002000,
//     VK_SHADER_STAGE_TASK_BIT_EXT = 0x00000040,
//     VK_SHADER_STAGE_MESH_BIT_EXT = 0x00000080,
//     VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI = 0x00004000,
//     VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI = 0x00080000,
//     VK_SHADER_STAGE_RAYGEN_BIT_NV = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
//     VK_SHADER_STAGE_ANY_HIT_BIT_NV = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
//     VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
//     VK_SHADER_STAGE_MISS_BIT_NV = VK_SHADER_STAGE_MISS_BIT_KHR,
//     VK_SHADER_STAGE_INTERSECTION_BIT_NV = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
//     VK_SHADER_STAGE_CALLABLE_BIT_NV = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
//     VK_SHADER_STAGE_TASK_BIT_NV = VK_SHADER_STAGE_TASK_BIT_EXT,
//     VK_SHADER_STAGE_MESH_BIT_NV = VK_SHADER_STAGE_MESH_BIT_EXT,
//     VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
// } VkShaderStageFlagBits;
      static std::unordered_map<shader::type, VkShaderStageFlagBits> const shaderbit_lut = {
	 {shader::e_vertex, VK_SHADER_STAGE_VERTEX_BIT},
	 {shader::e_hull, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
	 {shader::e_domain, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
	 {shader::e_geomtry, VK_SHADER_STAGE_GEOMETRY_BIT},
	 {shader::e_fragment, VK_SHADER_STAGE_FRAGMENT_BIT},
	 {shader::e_task, VK_SHADER_STAGE_TASK_BIT_EXT},
	 {shader::e_mesh, VK_SHADER_STAGE_MESH_BIT_EXT},
	 {shader::e_compute, VK_SHADER_STAGE_COMPUTE_BIT},
      };

      VkShaderModule CreateShaderModule(const std::vector<char> &code, VkDevice logical_device)
      {
	 // todo: this should have a cache and we should hash the &code.
	 // ----: return the shader module if found or assert if we ever 
	 // ----: try to recreate a shader module.
	 log::debug("CreateShaderModule");
	 VkShaderModuleCreateInfo create_info = {};
	 create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	 create_info.codeSize = code.size();
	 create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

	 VkShaderModule shader_module;
	 if (vkCreateShaderModule(logical_device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to create shader module!");
	 }
	 return shader_module;
      }
   }
   namespace pipeline
   {
      VkPipelineVertexInputStateCreateInfo GetDefaultVertexInputState()
      {
	 // note: these are static because they're passed by ref below.
	 // ----: similar should be done if a non default vertex input state func is made.
	 static VkVertexInputBindingDescription vert_binding_description = vkVertex::GetBindingDescription();
	 static std::array<VkVertexInputAttributeDescription, 4> vert_attribute_description = vkVertex::GetAttributeDescriptions();
	 
	 VkPipelineVertexInputStateCreateInfo vertex_input_ci = {};
	 vertex_input_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	 vertex_input_ci.vertexBindingDescriptionCount = 1;
	 vertex_input_ci.pVertexBindingDescriptions = &vert_binding_description;
	 vertex_input_ci.vertexAttributeDescriptionCount = vert_attribute_description.size();
	 vertex_input_ci.pVertexAttributeDescriptions = vert_attribute_description.data();
	 return vertex_input_ci;
      }
      VkPipelineRasterizationStateCreateInfo GetDefaultRasterState()
      {
	 VkPipelineRasterizationStateCreateInfo rasteriser_ci = {};
	 rasteriser_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	 rasteriser_ci.depthClampEnable = VK_FALSE;
	 rasteriser_ci.polygonMode = VK_POLYGON_MODE_FILL;
	 rasteriser_ci.lineWidth = 1.0f;
	 // note: VK_CULL_MODE_NONE for debugging culling.
	 rasteriser_ci.cullMode = VK_CULL_MODE_BACK_BIT;
	 rasteriser_ci.frontFace = VK_FRONT_FACE_CLOCKWISE;
	 rasteriser_ci.depthBiasEnable = VK_FALSE;
	 rasteriser_ci.depthBiasConstantFactor = 0.0f;
	 rasteriser_ci.depthBiasClamp = 0.0f;
	 rasteriser_ci.depthBiasSlopeFactor = 0.0f;
	 return rasteriser_ci;
      }
      VkViewport GetDefaultViewport()
      {
	 auto& pass = g_pass_map["swapchain"];
	 VkViewport viewport = {};
	 viewport.x = 0.0f;
	 viewport.y = 0.0f;
	 viewport.width = (float)pass.extent.width;
	 viewport.height = (float)pass.extent.height;
	 viewport.minDepth = 0.0f;
	 viewport.maxDepth = 1.0f;
	 return viewport;
      }
      VkRect2D GetDefaultScissor()
      {
	 auto& pass = g_pass_map["swapchain"];
	 VkRect2D scissor = {};
	 scissor.offset = {0, 0};
	 scissor.extent = pass.extent;
	 return scissor;
      }
      VkPipelineViewportStateCreateInfo GetDefaultViewportState()
      {
	 // note: the statics are because they're passed by ref below.
	 // ----: similar should be done if a non default viewport state func is made.
	 static VkViewport viewport = {};
	 viewport = GetDefaultViewport();
	 
	 static VkRect2D scissor = {};
	 scissor = GetDefaultScissor();

	 VkPipelineViewportStateCreateInfo viewport_state_ci = {};
	 viewport_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	 viewport_state_ci.viewportCount = 1;
	 viewport_state_ci.pViewports = &viewport;
	 viewport_state_ci.scissorCount = 1;
	 viewport_state_ci.pScissors = &scissor;
	 return viewport_state_ci;
      }
      VkPipelineMultisampleStateCreateInfo GetDefaultMultisampleState()
      {
	 VkPipelineMultisampleStateCreateInfo multisample_ci = {};
	 multisample_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	 multisample_ci.sampleShadingEnable = VK_FALSE;
	 multisample_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	 multisample_ci.minSampleShading = 1.0f;
	 multisample_ci.pSampleMask = nullptr;
	 multisample_ci.alphaToCoverageEnable = VK_FALSE;
	 multisample_ci.alphaToOneEnable = VK_FALSE;
	 return multisample_ci;
      }
      VkPipelineInputAssemblyStateCreateInfo GetDefaultIAState()
      {
	 VkPipelineInputAssemblyStateCreateInfo input_create_info = {};
	 input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	 input_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	 input_create_info.primitiveRestartEnable = VK_FALSE;
	 return input_create_info;
      }
      VkPipelineColorBlendStateCreateInfo GetDefaultBlendState()
      {
	 static VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
	 color_blend_attachment_state.colorWriteMask =
	    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	 color_blend_attachment_state.blendEnable = VK_FALSE;
	 // blend is disabled so these options do nothing
	 // they're just an example of some simple blending parameters.
	 color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	 color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	 color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	 color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	 color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	 color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
	 
	 VkPipelineColorBlendStateCreateInfo color_blend_state_ci = {};
	 color_blend_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	 color_blend_state_ci.logicOpEnable = VK_FALSE;
	 // logic op is disabled so this line is optional.
	 color_blend_state_ci.logicOp = VK_LOGIC_OP_COPY;
	 //
	 color_blend_state_ci.attachmentCount = 1;
	 color_blend_state_ci.pAttachments = &color_blend_attachment_state;
	 // optional constants. I think these are actually used, they're just initialised to zero by defualt.
	 color_blend_state_ci.blendConstants[0] = 0.0f;
	 color_blend_state_ci.blendConstants[1] = 0.0f;
	 color_blend_state_ci.blendConstants[2] = 0.0f;
	 color_blend_state_ci.blendConstants[3] = 0.0f;
	 return color_blend_state_ci;
      }
      
      int CreatePBRPipelineVariants(Material mat)
      {
	 log::debug("CreatePBRGraphicsPipeline");
	 auto hash = hash::i32((const char*)&mat, sizeof(Material));
	 if(g_pipe_map.find(hash) == g_pipe_map.end())
	 {
	    VkPipelineLayout pipeline_layout;
	    auto pipeline_layout_ci = GetPBRPipelineLayout();
	    if (vkCreatePipelineLayout(g_logical_device, &pipeline_layout_ci, nullptr, &pipeline_layout) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create pipeline layout!");
	    }
	    unsigned int shader_count = 0;
	    std::array<VkPipelineShaderStageCreateInfo,fw::shader::e_count> shader_stage_ci = {};
	    VkShaderModule vertex_shader = {};
	    for(int i = 0; i < fw::shader::e_count; i++)
	    {
	       if(mat[i].is_valid())
	       {
		  if(auto module = g_shaders[i].find(mat[i]); module != g_shaders[i].end())
		  {

		     VkPipelineShaderStageCreateInfo& stage_ci = shader_stage_ci[shader_count++];
		     stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		     stage_ci.stage = shaders::shaderbit_lut.find((shader::type)i)->second;
		     stage_ci.module = module->second;
		     stage_ci.pName = "main";
		     if((shader::type)i == shader::e_vertex) { vertex_shader = stage_ci.module; }
		  }
	       }
	    }

	    assert(shader_count != 0);
	    auto vertex_input_ci = GetDefaultVertexInputState();
	    auto input_assembly_ci = GetDefaultIAState();
	    auto viewport_state_ci = GetDefaultViewportState();
	    auto raster_state_ci = GetDefaultRasterState();
	    auto multisample_state_ci = GetDefaultMultisampleState();
	    auto blend_state_ci = GetDefaultBlendState();

	    // Setting any of these means parts of the associated create info is ignored.
	    // The related parts must be recorded in commands, listing related commands below
	    std::vector<VkDynamicState> dynamic_states = {
	       VK_DYNAMIC_STATE_VIEWPORT, // vkCmdSetViewport
	       VK_DYNAMIC_STATE_SCISSOR,  // vkCmdSetScissor
	       // so we can set this to false if we don't want to write depth.
	       // VK_DYNAMIC_STATE_STENCIL_WRITE_MASK // vkCmdSetStencilWriteMask()
	    };

	    VkPipelineDynamicStateCreateInfo dynamic_state_ci = {};
	    dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	    dynamic_state_ci.dynamicStateCount = dynamic_states.size();
	    dynamic_state_ci.pDynamicStates = dynamic_states.data();

	    VkPipelineDepthStencilStateCreateInfo depth_stencil_ci = {};
	    depth_stencil_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	    // todo: need a separate set of pipeline states without depth write, only depth test.
	    // ----: or not, VkPileineDynamicState lets us change VK_DYNAMIC_STATE_STENCIL_WRITE_MASK
	    depth_stencil_ci.depthTestEnable = VK_TRUE;
	    depth_stencil_ci.depthWriteEnable = VK_FALSE;
	    //
	    depth_stencil_ci.depthCompareOp = VK_COMPARE_OP_LESS;
	    depth_stencil_ci.depthBoundsTestEnable = VK_FALSE;
	    depth_stencil_ci.minDepthBounds = 0.0f; // Optional
	    depth_stencil_ci.maxDepthBounds = 1.0f; // Optional
	    depth_stencil_ci.stencilTestEnable = VK_FALSE;
	    depth_stencil_ci.front = {}; // Optional
	    depth_stencil_ci.back = {}; // Optional
	    
	    VkGraphicsPipelineCreateInfo pipeline_ci = {};
	    pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	    pipeline_ci.stageCount = shader_count;
	    pipeline_ci.pStages = shader_stage_ci.data();
	    pipeline_ci.pVertexInputState = &vertex_input_ci;
	    pipeline_ci.pInputAssemblyState = &input_assembly_ci;
	    pipeline_ci.pViewportState = &viewport_state_ci;
	    pipeline_ci.pRasterizationState = &raster_state_ci;
	    pipeline_ci.pMultisampleState = &multisample_state_ci;
	    pipeline_ci.pDepthStencilState = &depth_stencil_ci;
	    pipeline_ci.pColorBlendState = &blend_state_ci;
	    pipeline_ci.pDynamicState = &dynamic_state_ci;
	    pipeline_ci.layout = pipeline_layout;
	    // note: this pipeline isn't limited to this render pass.
	    // ----: but we do require a render pass, so setup a default.
	    pipeline_ci.renderPass = g_pass_map["swapchain"].pass;
	    pipeline_ci.subpass = 0;
	    pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
	    pipeline_ci.basePipelineIndex = -1;

	    log::debug("CreateRaster");
	    VkPipeline graphics_pipeline;
	    if (vkCreateGraphicsPipelines(g_logical_device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr,
					  &graphics_pipeline) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create graphics pipeline!");
	    }
	    VkPipeline depth_pipeline = {};
	    if(vertex_shader)
	    {
	       depth_stencil_ci.depthWriteEnable = VK_TRUE;
	       // pipeline_ci.pDepthStencilState = &depth_stencil_ci;
	       // todo: see if this matters? should this be reset.
	       // pipeline_ci.pRasterizationState = nullptr;
	       // pipeline_ci.pMultisampleState = nullptr;
	       // pipeline_ci.pColorBlendState = nullptr;
	       // pipeline_ci.pDynamicState = nullptr;
	       pipeline_ci.stageCount = 1;
	       VkPipelineShaderStageCreateInfo stage_ci = {};
	       stage_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	       stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
	       stage_ci.module = vertex_shader;
	       stage_ci.pName = "main";
	       log::debug("CreateDepth");
	       pipeline_ci.pStages = &stage_ci;
	       if (vkCreateGraphicsPipelines(g_logical_device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr,
					     &depth_pipeline) != VK_SUCCESS)
	       {
		  throw std::runtime_error("failed to create graphics pipeline!");
	       }
	    }
	    g_pipe_map[hash] = {graphics_pipeline, depth_pipeline, pipeline_layout};
	    log::debug("pipeline created id: {}", hash);
	 }
	 else
	 {
	    log::debug("pipeline reuse id: {}", hash);
	 }

	 return hash;
      }
   }
   namespace renderpass
   {
      void RecordPass(hash::string passname)
      {
	 PassHandle& pass = g_pass_map[passname];

	 VkCommandBufferBeginInfo begin_info = {};
	 begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	 begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	 begin_info.pInheritanceInfo = nullptr;
	 
	 if (vkBeginCommandBuffer(pass.cmd_buffer, &begin_info) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to begin recording command buffer!");
	 }

	 VkViewport vp[] = {pipeline::GetDefaultViewport()};
	 vkCmdSetViewport(pass.cmd_buffer, 0, 1, vp);
	 VkRect2D scissor[] = {pipeline::GetDefaultScissor()};
	 vkCmdSetScissor(pass.cmd_buffer, 0, 1, scissor);
	 

	 VkRenderPassBeginInfo render_pass_begin_info = {};
	 render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	 render_pass_begin_info.renderPass = pass.pass;
         render_pass_begin_info.framebuffer = pass.CurrentFrame();
	 render_pass_begin_info.renderArea.offset = {0, 0};
	 render_pass_begin_info.renderArea.extent = pass.extent;

	 std::array<VkClearValue, 2> clear_values{};
	 // clear_colour - comment because I keep failing to find this.
	 clear_values[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
	 clear_values[1].depthStencil = {1.0f, 0};

	 render_pass_begin_info.clearValueCount = clear_values.size();
	 render_pass_begin_info.pClearValues = clear_values.data();
	 
	 // log::debug("begin renderpass");
	 vkCmdBeginRenderPass(pass.cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	 
	 uint32_t pipeline_hash = 0;
	 uint32_t id = 0;
	 for(auto dh : pass.draws)
	 {
	    // todo: push constants have limited mat4x4s atm, fix it.
	    assert(id < 16);
	    // log::debug("Recording Draw vb: {} pi: {} nverts: {}", dh.vb_handle, dh.pi_handle, dh.num_verts);
	    // log::debug("bind pipeline");

	    if(uint32_t hash = dh.pi_handle; hash != pipeline_hash)
	    {
	       // note: setting viewport and scissor when we bind pipeline incase it's changed
	       // todo: it would be nicer to have this sort of thing covered as a init frame pass or something.
	       pipeline_hash = hash;
	       // todo: this works because we only really have 1 pipeline layout.
	       vkCmdBindDescriptorSets(pass.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_map[pipeline_hash].layout, 0, 1, &g_shared_descriptor_sets[g_flight_frame], 0, nullptr);
	    }
	    
	    VkBuffer vertex_buffers[] = {g_vb_map[dh.vb_handle].vb};
	    VkDeviceSize offsets[] = {0};
	    DefaultPushConstants constants = {id};
	    // todo: make this happen once per draw before depth when recording depth
	    memcpy(&ubo.model[id], &dh.owner->transform, sizeof(mat4x4f));
	    // this needs to happen every time we setup a draw.
	    
	    vkCmdPushConstants(pass.cmd_buffer, g_pipe_map[pipeline_hash].layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DefaultPushConstants), &constants);
	    vkCmdBindVertexBuffers(pass.cmd_buffer, 0, 1, vertex_buffers, offsets);
	    auto ibh = g_ib_map[dh.ib_handle];
	    vkCmdBindIndexBuffer(pass.cmd_buffer, ibh.ib, 0, VK_INDEX_TYPE_UINT16);

	    assert(dh.ds_handle != -1);
	    
	    // std::array<VkDescriptorSet, 2> desc_sets = {g_descriptor_sets[g_flight_frame], g_drawdescriptor_sets[dh.ds_handle] };

	    // note: the firstSet value is 1, because we're binding from that set number. I.e. we're binding set 1, which has our per-draw descriptor layout. (image, image)
	    // ----: vkCmdBindDescriptorSets(pass.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_map[pipeline_hash].layout, 0, 2, desc_sets.data(), 0, nullptr);
	    vkCmdBindDescriptorSets(pass.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_map[pipeline_hash].layout, 1, 1, &g_pbr_descriptor_sets[dh.ds_handle], 0, nullptr);
	    
	    vkCmdBindPipeline(pass.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_map[pipeline_hash].pipeline);
	    vkCmdDrawIndexed(pass.cmd_buffer, ibh.len, 1, 0, 0, 0);
	    vkCmdBindPipeline(pass.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_map[pipeline_hash].depth_pipeline);
	    vkCmdDrawIndexed(pass.cmd_buffer, ibh.len, 1, 0, 0, 0);
	    id++;
	 }

	 // log::debug("end renderpass");
	 vkCmdEndRenderPass(pass.cmd_buffer);
	 
	 // log::debug("end commandbuffer");
	 if (vkEndCommandBuffer(pass.cmd_buffer) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to record command buffer!");
	 }

      }
   }
   namespace barriers
   {
      bool CreatePassSemaphores(hash::string passname)
      {
	 log::debug("CreateSemaphores");
	 if(auto it = g_semaphore_map.find(passname); it == g_semaphore_map.end())
	 {
	    auto& semas = g_semaphore_map[passname];
	    semas.image_available.resize(g_max_frames_in_flight);
	    semas.render_finished.resize(g_max_frames_in_flight);
	    semas.in_flight_fences.resize(g_max_frames_in_flight);

	    VkSemaphoreCreateInfo semaphore_create_info = {};
	    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	    VkFenceCreateInfo fence_create_info = {};
	    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	    for (int i = 0; i < g_max_frames_in_flight; i++)
	    {
	       if (vkCreateSemaphore(g_logical_device, &semaphore_create_info, nullptr, &semas.image_available[i]) !=
		   VK_SUCCESS)
	       {
		  throw std::runtime_error("failed to create semaphore!");
	       }

	       if (vkCreateSemaphore(g_logical_device, &semaphore_create_info, nullptr, &semas.render_finished[i]) !=
		   VK_SUCCESS)
	       {
		  throw std::runtime_error("failed to create semaphore!");
	       }

	       if (vkCreateFence(g_logical_device, &fence_create_info, nullptr, &semas.in_flight_fences[i]) != VK_SUCCESS)
	       {
		  throw std::runtime_error("failed to create semaphore!");
	       }
	    }
	    return true;
	 }
	 return false;
      }
   }
} // namespace fwvulkan
mat4x4f g_view;
vec3f g_cam_pos;
fw::Light g_light;
void UpdateUniformBuffer(uint32_t currentImage)
{
   using namespace fwvulkan;
   auto extent = g_pass_map["swapchain"].extent;
   // todo: move this matrix into the camera probably.
   ubo.proj.perspective(60.0f, (float)extent.width / extent.height, 0.1f, 100.f);
   ubo.view = g_view;
   
   ubo.light = g_light.position;
   ubo.cam_pos = g_cam_pos;

   ubo.light_intensity = g_light.intensity;
   memcpy(g_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

int gGlfwVulkan::init()
{
   using namespace fwvulkan;
   log::topics::add("gGlfwVulkan");
   log::scope topic("gGlfwVulkan");
   log::debug("Init Vulkan");
   params::get_value("vulkan.validation", g_enable_validation_layers);
   log::debug("enable validaiton layers: {}", g_enable_validation_layers);
   instance::CreateInstance();
   if (g_enable_validation_layers)
   {
      instance::SetupDebugMessenger();
   }
   if (g_window != nullptr)
   {
      instance::CreateSurface();
   }
   device::PickPhysicalDevice();

   // SwapChainSupportDetails swap_chain_support = device::QuerySwapChainSupport(g_physical_device, g_surface);
   // VkSurfaceFormatKHR surface_format = swapchain::ChooseSwapSurfaceFormat(swap_chain_support.formats);
   // swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, int *window);
   // VkPresentModeKHR present_mode = ChooseSwapPresentMode(swap_chain_support.present_modes);
   // VkExtent2D extent = ChooseSwapExtent(swap_chain_support.capabilites, g_window);
   
   device::CreateLogicalDevice();
   swapchain::CreateCommandPool();

   // todo: this sucks
   // ----: better would be to do all of this at the point of shader/material registration
   // ----: register the material, it lists the shaders, which list the sets / binds.
   // ----: store descriptor sets / layouts / pools with the pipelines
   // ----: have them in a hashed map for reuse.
   {
      buffers::CreateDefaultUniformBuffers();
      {
	 VkDescriptorType types[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_SAMPLER };
	 VkShaderStageFlags stages[] = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
	 g_shared_descriptor_set_layout = buffers::CreateDescriptorSetLayout(types, stages, 2);
	 g_shared_descriptor_pool = buffers::CreateDescriptorPool();
	 buffers::CreateDescriptorSets();
      }
      buffers::InitPBRDescriptors();
   }
   // todo: swapchain pass extent is set here, kinda gross.
   swapchain::CreateSwapChain();
   // todo: this sucks. :) 
   renderpass::CreateDepthBuffer();
   swapchain::CreateSwapchainImageViews();
   swapchain::CreateSwapchainFrameBuffers();
   barriers::CreatePassSemaphores("swapchain");
   return 0;
}
hash::string shaders[shader::e_count];
void gGlfwVulkan::visit(class physics::collider::polygon * /*_poly*/) {}
#include "../camera/camera.h"
void gGlfwVulkan::visit(camera* _camera)
{
   g_view = _camera->getView();
   g_cam_pos = _camera->getPosition();
}

void gGlfwVulkan::visit(fw::Mesh* _mesh)
{
   using namespace fwvulkan;
   DrawHandle drawhandle;
   //todo: don't store drawhandles against their ptr address.
   auto handles_iter = g_drawhandles.find(_mesh);
   if(handles_iter == g_drawhandles.end())
   {
      log::debug("Visitor Mesh: {}", (void*)_mesh);
      drawhandle = { _mesh,
	 buffers::CreateVertexBufferHandle(_mesh->geometry.vbo.data, _mesh->geometry.vbo.len),
	 buffers::CreateIndexBufferHandle(_mesh->geometry.ibo.data, _mesh->geometry.ibo.len),
	 // todo: this needs to handle no images
	 // ----: kindof done. draw descriptors default to safe image views.
	 {},
	 // todo: this should be "create all pipeline variants"
	 // ----: then we store draws against pipelines / passes, or something. :)
	 pipeline::CreatePBRPipelineVariants(_mesh->material),
	 g_used_pbr_descriptors++
      };
      log::debug("draw descriptors: {}/{}", g_used_pbr_descriptors, g_pbr_descriptor_sets.size());
      assert(size_t(g_used_pbr_descriptors) <  g_pbr_descriptor_sets.size());
      std::vector<VkDescriptorSet> set(1, {g_pbr_descriptor_sets[drawhandle.ds_handle]});
      
      for(unsigned int i = 0; i < Mesh::max_images; i++)
      {
	 if(_mesh->images[i].data == nullptr) break;
	 drawhandle.im_handles[i] = buffers::CreateImageHandle(_mesh->images[i].data, _mesh->images[i].width, _mesh->images[i].height);
	 if(i == 0) buffers::SetPBRDescriptorAlbedo(g_im_map[drawhandle.im_handles[0]].view, set);
	 else if(i == 1) buffers::SetPBRDescriptorRoughness(g_im_map[drawhandle.im_handles[1]].view, set);
      }
      g_drawhandles[_mesh] = drawhandle;
   }
   else { drawhandle = handles_iter->second; }
   for (auto pass : _mesh->passes)
   {
      g_pass_map[pass].draws.push_back(drawhandle);
   }
}

void gGlfwVulkan::visit(fw::Light* _light)
{
   g_light = *_light;
}

int gGlfwVulkan::shutdown()
{
   using namespace fwvulkan;
   log::scope topic("gGlfwVulkan");
   log::debug("shutdown");
   swapchain::CleanupSwapChain();
   for(auto vb : g_vb_map)
   {
      vkDestroyBuffer(g_logical_device, vb.second.vb, nullptr);
      vkFreeMemory(g_logical_device, vb.second.vb_mem, nullptr);
   }
   for(auto ib : g_ib_map)
   {
      vkDestroyBuffer(g_logical_device, ib.second.ib, nullptr);
      vkFreeMemory(g_logical_device, ib.second.ib_mem, nullptr);
   }
   for(auto im : g_im_map)
   {
      vkDestroyImage(g_logical_device, im.second.image, nullptr);
      vkDestroyImageView(g_logical_device, im.second.view, nullptr);
      vkFreeMemory(g_logical_device, im.second.image_mem, nullptr);
   }
   for(auto pipeline : g_pipe_map)
   {
      vkDestroyPipeline(g_logical_device, pipeline.second.pipeline, nullptr);
      vkDestroyPipelineLayout(g_logical_device, pipeline.second.layout, nullptr);
   }
   g_pipe_map.clear();

   for(auto sema : g_semaphore_map)
   {
      for (int i = 0; i < g_max_frames_in_flight; i++)
      {
	 vkDestroySemaphore(g_logical_device, sema.second.image_available[i], nullptr);
	 vkDestroySemaphore(g_logical_device, sema.second.render_finished[i], nullptr);
	 vkDestroyFence(g_logical_device, sema.second.in_flight_fences[i], nullptr);
      }
   }
   vkDestroyCommandPool(g_logical_device, g_command_pool, nullptr);
   for(int t = fw::shader::e_fragment; t != fw::shader::e_count; t++)
   {
      for(auto s : g_shaders[(fw::shader::type)t])
      {
	 vkDestroyShaderModule(g_logical_device, s.second, nullptr);
      }
   }
   
   for (size_t i = 0; i < g_max_frames_in_flight; i++)
   {
      vkDestroyBuffer(g_logical_device, g_uniformBuffers[i], nullptr);
      vkFreeMemory(g_logical_device, g_uniformBuffersMemory[i], nullptr);
   }
   
   for (auto sam : g_sam_map)
   {
      vkDestroySampler(g_logical_device, sam.second.sampler, nullptr);
   }
   g_sam_map.clear();
   
   vkDestroyDescriptorPool(g_logical_device, g_shared_descriptor_pool, nullptr);
   vkDestroyDescriptorPool(g_logical_device, g_pbr_descriptor_pool, nullptr);
   vkDestroyDescriptorSetLayout(g_logical_device, g_shared_descriptor_set_layout, nullptr);
   vkDestroyDescriptorSetLayout(g_logical_device, g_pbr_descriptor_set_layout, nullptr);
   
   vkDestroyDevice(g_logical_device, nullptr);
   if (g_enable_validation_layers)
   {
      instance::DestroyDebugUtilsMessengerEXT(g_instance, g_debug_messenger, nullptr);
   }
   if (g_window != nullptr)
   {
      vkDestroySurfaceKHR(g_instance, g_surface, nullptr);
   }

   return 0;
}
int gGlfwVulkan::update() { return 0; }
int gGlfwVulkan::render()
{
   using namespace fwvulkan;

   for(auto pass : g_pass_map)
   {
      vkWaitForFences(g_logical_device, 1, &g_semaphore_map[pass.first].in_flight_fences[g_flight_frame], VK_TRUE, std::numeric_limits<uint64_t>::max());
   }

   // this is needed for present, it's not needed for graphics queue submit.
   uint32_t image_index = 0;
   {
      VkResult result = vkAcquireNextImageKHR(g_logical_device, g_swap_chain, std::numeric_limits<uint64_t>::max(),
					      g_semaphore_map["swapchain"].image_available[g_flight_frame], VK_NULL_HANDLE, &image_index);
      if (result == VK_ERROR_OUT_OF_DATE_KHR || g_resized)
      {
	 g_resized = false;
	 swapchain::RecreateSwapChain();
	 return 0;
      }
      else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      {
	 throw std::runtime_error("failed to acquire swap chain image!");
      }
   }
   UpdateUniformBuffer(g_flight_frame);
   for(auto pass : g_pass_map)
   {
      fwvulkan::renderpass::RecordPass(pass.first);
   }
   std::vector<VkSemaphore> all_signals;
   for(auto pass : g_pass_map)
   {
      VkSemaphore wait_semaphores[] = {g_semaphore_map[pass.first].image_available[g_flight_frame]};
      VkSemaphore signals[] = {g_semaphore_map[pass.first].render_finished[g_flight_frame]};
      VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
      VkCommandBuffer cmd_buffers[] = {g_pass_map[pass.first].cmd_buffer};
      
      VkSubmitInfo submit_info = {};
      submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      // todo: find out why vkQueueSubmit hangs if we add waits for non swapchain.
      // ----: Ok, so the vkAcquireNextImageKHR triggers the image_available semaphore, which allows submission/execution.
      // todo: define better waits per pass.
      submit_info.waitSemaphoreCount = pass.first == hash::string("swapchain") ? 1 : 0;
      // todo: this is probably where we want to add waits for pass dependencies
      // ----: e.g. wait for depth pass to complete before we run colour
      submit_info.pWaitSemaphores = pass.first == hash::string("swapchain") ? wait_semaphores : nullptr;
      submit_info.pWaitDstStageMask = wait_stages;
      submit_info.commandBufferCount = 1;
      submit_info.pCommandBuffers = cmd_buffers;
      // note: this triggers when all commands are complete.
      submit_info.signalSemaphoreCount = 1;
      submit_info.pSignalSemaphores = signals;
      
      all_signals.push_back(signals[0]);

      vkResetFences(g_logical_device, 1, &g_semaphore_map[pass.first].in_flight_fences[g_flight_frame]);
      if (vkQueueSubmit(g_graphics_queue, 1, &submit_info, g_semaphore_map[pass.first].in_flight_fences[g_flight_frame]) != VK_SUCCESS)
      {
	 throw std::runtime_error("failed to submit draw command buffer!");
      }
   }
   // handle present
   {
      VkPresentInfoKHR present_info = {};
      present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
      present_info.waitSemaphoreCount = all_signals.size();
      present_info.pWaitSemaphores = all_signals.data();
      VkSwapchainKHR swap_chains[] = {g_swap_chain};
      present_info.swapchainCount = 1;
      present_info.pSwapchains = swap_chains;
      present_info.pImageIndices = &image_index;
      present_info.pResults = nullptr;
      VkResult result = vkQueuePresentKHR(g_present_queue, &present_info);
      if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
      {
	 swapchain::RecreateSwapChain();
      }
      else if (result != VK_SUCCESS)
      {
	 throw std::runtime_error("failed to present swap chain image!");
      }
   }
   g_current_frame++;
   g_flight_frame = g_current_frame % g_max_frames_in_flight;
   vkResetCommandPool(g_logical_device, g_command_pool, 0);
   for(auto& pass : g_pass_map)
   {
      pass.second.draws.clear();
   }
   return 0;
}


#include <fstream>
static std::vector<char> read_file(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed ot open " + filename);
    }

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    
    return buffer;
}

// Graphics
// VK_PIPELINE_BIND_POINT_GRAPHICS
//   VK_SHADER_STAGE_VERTEX_BIT
//   VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
//   VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
//   VK_SHADER_STAGE_GEOMETRY_BIT
//   VK_SHADER_STAGE_FRAGMENT_BIT
//   VK_SHADER_STAGE_TASK_BIT_EXT
//   VK_SHADER_STAGE_MESH_BIT_EXT

// compute
// VK_PIPELINE_BIND_POINT_COMPUTE
//   VK_SHADER_STAGE_COMPUTE_BIT

bool gGlfwVulkan::register_shader(fw::hash::string name, const char* path, fw::shader::type type)
{
   using namespace fwvulkan;
   auto shader_code = read_file(path);
   auto shader = shaders::CreateShaderModule(shader_code, g_logical_device);
   g_shaders[type][name] = shader;
   return true;
}

bool gGlfwVulkan::register_pass(fw::hash::string pass)
{
   using namespace fwvulkan;
   PassHandle& swapchain = g_pass_map["swapchain"];
   if(renderpass::CreateDefaultRenderPass(pass, swapchain.extent, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL))
   {
      renderpass::CreatePassImages(pass);
      barriers::CreatePassSemaphores(pass);
      return true;
   }
   return false;
}

iRenderVisitor* gGlfwVulkan::getRenderer()
{
   return (iRenderVisitor*)this;
}

graphics* graphics::graphicsFactory()
{
   return (graphics*)new gGlfwVulkan;
}
