#include "../graphics2.h"
namespace fwvulkan
{
   struct VBHandle;
   extern VkDevice g_logical_device;
   extern VkDescriptorSetLayout g_shared_descriptor_set_layout;
   extern std::map<uint32_t, struct IMHandle> g_im_map;
   extern std::map<fw::hash::string, struct PassHandle> g_pass_map;

   namespace renderpass
   {
      VkImageView GetPassImageView(fw::hash::string passname);
   }
   namespace buffers
   {
      extern fw::HandlePtr<IMHandle> CreateImageHandle(fw::Image& image);
      extern fw::HandlePtr<VBHandle> CreateVertexBufferHandle(const fw::Vertex* vertices, int num_vertices);
      extern fw::HandlePtr<IBHandle> CreateIndexBufferHandle(const uint32_t* indices, int num_indices);
      extern void SetDescriptorImage(VkImageView image_view, std::vector<VkDescriptorSet> image_sets, unsigned int dst_binding);
      extern VkDescriptorSetLayout CreateDescriptorSetLayout(VkDescriptorType* types, VkShaderStageFlags* stage_flags, VkDescriptorBindingFlags* bind_flags, int num);
   }
   namespace pipeline
   {
      extern fw::HandlePtr<PipelineHandle> CreatePipelineVariants(fw::Material mat, VkPipelineLayoutCreateInfo pipeline_layout_ci);
   }
}
