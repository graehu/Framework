#include "gGlfwVulkan.h"
#include "../../../../Libs/Vulkan/1.3.280.1/x86_64/include/vulkan/vulkan.hpp"
#include "../../../../Libs/glfw-3.4/include/GLFW/glfw3.h"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <optional>
#include <array>
#include <vector>
#include "../../utils/log/log.h"
#include "../../utils/params.h"


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
   static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
   {
      std::array<VkVertexInputAttributeDescription, 2> attribute_description = {};
      attribute_description[0].binding = 0;
      attribute_description[0].location = 0;
      attribute_description[0].format = VK_FORMAT_R32G32_SFLOAT;
      attribute_description[0].offset = offsetof(Vertex, position);

      attribute_description[1].binding = 0;
      attribute_description[1].location = 1;
      attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_description[1].offset = offsetof(Vertex, color);

      return attribute_description;
   }
};
namespace fwvulkan
{
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
   // semaphores
   std::vector<VkSemaphore> g_image_available_semaphores;
   std::vector<VkSemaphore> g_render_finished_semaphores;
   std::vector<VkFence> g_in_flight_fences;
   const int g_max_frames_in_flight = 2;
   unsigned int g_current_frame = 0;
   unsigned int g_flight_frame = 0;
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

   };
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
      VkPipelineLayout layout = VK_NULL_HANDLE;
   };
   std::map<uint32_t, PipelineHandle> g_pipe_map;
   
   // commands
   VkCommandPool g_command_pool = VK_NULL_HANDLE;
   // std::vector<VkCommandBuffer> g_command_buffers;
   // vertex buffers
   struct VBHandle
   {
      VkBuffer vb;
      VkDeviceMemory vb_mem;
   };
   std::map<uint32_t, VBHandle> g_vb_map;
   // mesh
   std::vector<Mesh*> g_meshes;
   struct DrawHandle
   {
      int vb_handle = -1;
      int num_verts = 0;
      int pi_handle = -1;
   };
   std::map<fw::Mesh *, DrawHandle> g_drawhandles;
   //
   bool g_enable_validation_layers = false;
   const std::vector<const char*> g_instance_extensions = {
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
   };
   const std::vector<const char *> g_device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      // VK_KHR_DYNAMIC_RENDERING_NAME,
      VK_EXT_MEMORY_BUDGET_EXTENSION_NAME
   };
   // Note: These layers require you to run Vulkan/1.3.280.1/setup-env.sh prior to running the executable.
   const std::vector<const char *> g_validation_layers = {
      "VK_LAYER_KHRONOS_validation",
      // "VK_LAYER_RENDERDOC_Capture",
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
	 // todo: find out why renderdoc dies here. (glfwGetRequiredInstanceExtensions)
	 glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	 std::vector<const char *> required_extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	 if (g_enable_validation_layers)
	 {
	    required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
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
	    VkImageView attachments[] = {pass.image_views[i]};
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
	    VkImageViewCreateInfo create_info = {};
	    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	    create_info.image = pass.images[i];
	    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	    create_info.format = pass.image_format;

	    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	    create_info.subresourceRange.baseMipLevel = 0;
	    create_info.subresourceRange.levelCount = 1;
	    create_info.subresourceRange.baseArrayLayer = 0;
	    create_info.subresourceRange.layerCount = 1;

	    if (vkCreateImageView(g_logical_device, &create_info, nullptr, &pass.image_views[i]) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create image views!");
	    }
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
	 auto& pass = g_pass_map["swapchain"];
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

	 vkGetSwapchainImagesKHR(g_logical_device, g_swap_chain, &image_count, nullptr);
	 pass.images.resize(image_count);
	 vkGetSwapchainImagesKHR(g_logical_device, g_swap_chain, &image_count, pass.images.data());

	 pass.extent = extent;
      }
   }
   namespace renderpass
   {
      // create pass rendertarget
      VkImage CreatePassImage(hash::string passname)
      {
	 auto& pass = g_pass_map[passname];
	 assert(pass.extent.width > 0 && pass.extent.height > 0);


	 VkImageCreateInfo imageInfo{};
	 imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	 imageInfo.imageType = VK_IMAGE_TYPE_2D;
	 imageInfo.extent.width = pass.extent.width;
	 imageInfo.extent.height = pass.extent.height;
	 imageInfo.extent.depth = 1;
	 imageInfo.mipLevels = 1;
	 imageInfo.arrayLayers = 1;
	 
	 imageInfo.format = pass.image_format;// VK_FORMAT_R8G8B8A8_UNORM;//format;
	 imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;//tiling;
	 imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	 // this is a rendertarget
	 imageInfo.usage =
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
	    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	    VK_IMAGE_USAGE_SAMPLED_BIT;
	 
	 imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	 imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	 imageInfo.flags = 0;
	 // todo: check on these.
	 imageInfo.queueFamilyIndexCount = 0;
	 imageInfo.pQueueFamilyIndices = nullptr;

	 
	 VkImage image = VK_NULL_HANDLE;
	 if (vkCreateImage(g_logical_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
	    throw std::runtime_error("failed to create image!");
	 }
	 return image;
      }

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
	 
	 for (size_t i = 0; i < pass.images.size(); i++)
	 {
	    pass.images[i] = CreatePassImage(passname);
	    
	    VkMemoryAllocateInfo alloc_info = {};
	    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	    
	    VkMemoryRequirements mem_requirements;
	    vkGetImageMemoryRequirements(g_logical_device, pass.images[i], &mem_requirements);
	    
	    alloc_info.allocationSize = mem_requirements.size;
	    alloc_info.memoryTypeIndex = utils::FindMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	    if (vkAllocateMemory(g_logical_device, &alloc_info, nullptr, &pass.image_mems[i]) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to allocate image mem!");
	    }
	    if (vkBindImageMemory(g_logical_device, pass.images[i], pass.image_mems[i], 0) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to failed to bind image mem!");
	    }

	    VkImageViewCreateInfo create_info = {};
	    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	    create_info.image = pass.images[i];
	    create_info.pNext = nullptr;
	    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	    create_info.format = pass.image_format;
	    create_info.flags = 0;

	    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	    create_info.subresourceRange.baseMipLevel = 0;
	    create_info.subresourceRange.levelCount = 1;
	    create_info.subresourceRange.baseArrayLayer = 0;
	    create_info.subresourceRange.layerCount = 1;

	    if (vkCreateImageView(g_logical_device, &create_info, nullptr, &pass.image_views[i]) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create image views!");
	    }
	    
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
      bool CreateDefaultRenderPass(hash::string passname)
      {
	 if(g_pass_map.find(passname) == g_pass_map.end())
	 {
	    log::debug("Creating Default Renderpass: '{}'", passname.m_literal);
	    SwapChainSupportDetails swap_chain_support = device::QuerySwapChainSupport(g_physical_device, g_surface);
	    VkSurfaceFormatKHR surface_format = swapchain::ChooseSwapSurfaceFormat(swap_chain_support.formats);
	    
	    VkAttachmentDescription color_attachement = {};
	    color_attachement.format = surface_format.format;
	    color_attachement.samples = VK_SAMPLE_COUNT_1_BIT;
	    color_attachement.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	    color_attachement.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	    color_attachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	    color_attachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	    color_attachement.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	    
	    if(passname == hash::string("swapchain"))
	    {
	       color_attachement.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	    }
	    else
	    {
	       color_attachement.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	    }

	    VkAttachmentReference color_attachemnt_reference = {};
	    color_attachemnt_reference.attachment = 0;
	    color_attachemnt_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	    VkSubpassDescription subpass_description = {};
	    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	    subpass_description.colorAttachmentCount = 1;
	    subpass_description.pColorAttachments = &color_attachemnt_reference;

	    VkSubpassDependency subpass_dependency = {};

	    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	    subpass_dependency.dstSubpass = 0;
	    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	    VkRenderPassCreateInfo render_pass_create_info = {};
	    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	    render_pass_create_info.attachmentCount = 1;
	    render_pass_create_info.pAttachments = &color_attachement;
	    render_pass_create_info.subpassCount = 1;
	    render_pass_create_info.pSubpasses = &subpass_description;
	    render_pass_create_info.dependencyCount = 1;
	    render_pass_create_info.pDependencies = &subpass_dependency;
	    
	    VkRenderPass pass;
	    if (vkCreateRenderPass(g_logical_device, &render_pass_create_info, nullptr, &pass) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create render pass!");
	    }
	    
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
	    g_pass_map[passname] = {pass, buffer, {}, surface_format.format, {}, {}, {}, {}, {}};
	    return true;
	 }
	 return false;
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
	 renderpass::CreateDefaultRenderPass("swapchain");
	 CreateSwapChain();
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
	 static std::array<VkVertexInputAttributeDescription, 2> vert_attribute_description = vkVertex::GetAttributeDescriptions();
	 
	 VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {};
	 vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	 vertex_input_create_info.vertexBindingDescriptionCount = 1;
	 vertex_input_create_info.pVertexBindingDescriptions = &vert_binding_description;
	 vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vert_attribute_description.size());
	 vertex_input_create_info.pVertexAttributeDescriptions = vert_attribute_description.data();
	 return vertex_input_create_info;
      }
      VkPipelineRasterizationStateCreateInfo GetDefaultRasterState()
      {
	 VkPipelineRasterizationStateCreateInfo rasteriser_create_info = {};
	 rasteriser_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	 rasteriser_create_info.depthClampEnable = VK_FALSE;
	 rasteriser_create_info.polygonMode = VK_POLYGON_MODE_FILL;
	 rasteriser_create_info.lineWidth = 1.0f;
	 rasteriser_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
	 rasteriser_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	 rasteriser_create_info.depthBiasEnable = VK_FALSE;
	 rasteriser_create_info.depthBiasConstantFactor = 0.0f;
	 rasteriser_create_info.depthBiasClamp = 0.0f;
	 rasteriser_create_info.depthBiasSlopeFactor = 0.0f;
	 return rasteriser_create_info;
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

	 VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
	 viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	 viewport_state_create_info.viewportCount = 1;
	 viewport_state_create_info.pViewports = &viewport;
	 viewport_state_create_info.scissorCount = 1;
	 viewport_state_create_info.pScissors = &scissor;
	 return viewport_state_create_info;
      }
      VkPipelineMultisampleStateCreateInfo GetDefaultMultisampleState()
      {
	 VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
	 multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	 multisample_create_info.sampleShadingEnable = VK_FALSE;
	 multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	 multisample_create_info.minSampleShading = 1.0f;
	 multisample_create_info.pSampleMask = nullptr;
	 multisample_create_info.alphaToCoverageEnable = VK_FALSE;
	 multisample_create_info.alphaToOneEnable = VK_FALSE;
	 return multisample_create_info;
      }
      VkPipelineInputAssemblyStateCreateInfo GetDefaultIAState()
      {
	 VkPipelineInputAssemblyStateCreateInfo input_create_info = {};
	 input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	 input_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
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
	 
	  VkPipelineColorBlendStateCreateInfo color_blending_create_info = {};
	 color_blending_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	 color_blending_create_info.logicOpEnable = VK_FALSE;
	 // logic op is disabled so this line is optional.
	 color_blending_create_info.logicOp = VK_LOGIC_OP_COPY;
	 //
	 color_blending_create_info.attachmentCount = 1;
	 color_blending_create_info.pAttachments = &color_blend_attachment_state;
	 // optional constants. I think these are actually used, they're just initialised to zero by defualt.
	 color_blending_create_info.blendConstants[0] = 0.0f;
	 color_blending_create_info.blendConstants[1] = 0.0f;
	 color_blending_create_info.blendConstants[2] = 0.0f;
	 color_blending_create_info.blendConstants[3] = 0.0f;
	 return color_blending_create_info;
      }
      VkPipelineLayoutCreateInfo GetDefaultPipelineLayout()
      {
	 VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
	 pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	 pipeline_layout_create_info.setLayoutCount = 0;
	 pipeline_layout_create_info.pSetLayouts = nullptr;
	 pipeline_layout_create_info.pushConstantRangeCount = 0;
	 pipeline_layout_create_info.pPushConstantRanges = nullptr;
	 return pipeline_layout_create_info;
      }
      int CreateDefaultPipeline(Material mat)
      {
	 log::debug("CreateGraphicsPipeline");
	 auto hash = hash::i32((const char*)&mat, sizeof(Material));
	 if(g_pipe_map.find(hash) == g_pipe_map.end())
	 {
	    VkPipelineLayout pipeline_layout;
	    auto pipeline_layout_ci = GetDefaultPipelineLayout();
	    if (vkCreatePipelineLayout(g_logical_device, &pipeline_layout_ci, nullptr, &pipeline_layout) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create pipeline layout!");
	    }
	    unsigned int shader_count = 0;
	    VkPipelineShaderStageCreateInfo shader_create_infos[fw::shader::e_count] = {};
	    for(int i = 0; i < fw::shader::e_count; i++)
	    {
	       if(mat[i].is_valid())
	       {
		  if(auto module = g_shaders[i].find(mat[i]); module != g_shaders[i].end())
		  {
		     VkPipelineShaderStageCreateInfo& stage_create_info = shader_create_infos[shader_count++];
		     stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		     stage_create_info.stage = shaders::shaderbit_lut.find((shader::type)i)->second;
		     stage_create_info.module = module->second;
		     stage_create_info.pName = "main";
		  }
	       }
	    }
	    auto vertex_input_ci = GetDefaultVertexInputState();
	    auto input_assembly_ci = GetDefaultIAState();
	    auto viewport_state_ci = GetDefaultViewportState();
	    auto raster_state_ci = GetDefaultRasterState();
	    auto multisample_state_ci = GetDefaultMultisampleState();
	    auto blend_state_ci = GetDefaultBlendState();
	 
	    VkGraphicsPipelineCreateInfo pipeline_ci = {};
	    pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	    pipeline_ci.stageCount = shader_count;
	    pipeline_ci.pStages = shader_create_infos;
	    pipeline_ci.pVertexInputState = &vertex_input_ci;
	    pipeline_ci.pInputAssemblyState = &input_assembly_ci;
	    pipeline_ci.pViewportState = &viewport_state_ci;
	    pipeline_ci.pRasterizationState = &raster_state_ci;
	    pipeline_ci.pMultisampleState = &multisample_state_ci;
	    pipeline_ci.pDepthStencilState = nullptr;
	    pipeline_ci.pColorBlendState = &blend_state_ci;
	    pipeline_ci.pDynamicState = nullptr;
	    pipeline_ci.layout = pipeline_layout;
	    // note: this pipeline isn't limited to this render pass.
	    // ----: but we do require a render pass, so setup a default.
	    pipeline_ci.renderPass = g_pass_map["swapchain"].pass;
	    pipeline_ci.subpass = 0;
	    pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
	    pipeline_ci.basePipelineIndex = -1;
	 
	    VkPipeline graphics_pipeline;
	    if (vkCreateGraphicsPipelines(g_logical_device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr,
					  &graphics_pipeline) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create graphics pipeline!");
	    }
	    g_pipe_map[hash] = {graphics_pipeline, pipeline_layout};
	    log::debug("pipeline created id: {}", hash);
	 }
	 else
	 {
	    log::debug("pipeline reuse id: {}", hash);
	 }

	 return hash;
      }
   }
   namespace buffers
   {
      int CreateVertexBuffer(const fw::Vertex* vertices, int num_vertices)
      {
	 log::debug("Create Vertex Buffer");
	 uint32_t hash = hash::i32((const char*)vertices, num_vertices*sizeof(fw::Vertex));
	 if (g_vb_map.find(hash) == g_vb_map.end())
	 {
	    VkBufferCreateInfo buffer_create_info = {};
	    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	    buffer_create_info.size = sizeof(fw::Vertex) * num_vertices;
	    buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	    VkBuffer vertex_buffer;
	    if (vkCreateBuffer(g_logical_device, &buffer_create_info, nullptr, &vertex_buffer) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create vertex buffer!");
	    }
	    VkMemoryRequirements memory_requirements;
	    vkGetBufferMemoryRequirements(g_logical_device, vertex_buffer, &memory_requirements);

	    VkMemoryAllocateInfo allocate_info = {};
	    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	    allocate_info.allocationSize = memory_requirements.size;
	    allocate_info.memoryTypeIndex = utils::FindMemoryType(
	       memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	    VkDeviceMemory vertex_buffer_memory;
	    if (vkAllocateMemory(g_logical_device, &allocate_info, nullptr, &vertex_buffer_memory) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to allocate vertex buffer memory!");
	    }
	    vkBindBufferMemory(g_logical_device, vertex_buffer, vertex_buffer_memory, 0);
	    void *data;
	    vkMapMemory(g_logical_device, vertex_buffer_memory, 0, buffer_create_info.size, 0, &data);
	    memcpy(data, vertices, (size_t)buffer_create_info.size);
	    vkUnmapMemory(g_logical_device, vertex_buffer_memory);
	    g_vb_map[hash] = {vertex_buffer, vertex_buffer_memory};
	    log::debug("Created Vertex Buffer: {}", hash);
	 }
	 else
	 {
	    log::debug("Reusing Vertex Buffer: {}", hash);
	 }
	 return hash;
      }
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

	 VkRenderPassBeginInfo render_pass_begin_info = {};
	 render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	 render_pass_begin_info.renderPass = pass.pass;
	 // log::debug("pass {} fb {} extent {}, {}", passname.m_literal, (size_t)pass.CurrentFrame(), pass.extent.width, pass.extent.height);
         render_pass_begin_info.framebuffer = pass.CurrentFrame();
	 render_pass_begin_info.renderArea.offset = {0, 0};
	 render_pass_begin_info.renderArea.extent = pass.extent;

	 // log::debug("bound fb: {}", g_current_frame);
	 VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}}; // fixes a warning
	 render_pass_begin_info.clearValueCount = 1;
	 render_pass_begin_info.pClearValues = &clear_color;
	 
	 // log::debug("begin renderpass");
	 vkCmdBeginRenderPass(pass.cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	 uint32_t pipeline_hash = 0;
	 for(auto dh : pass.draws)
	 {
	    // log::debug("Recording Draw vb: {} pi: {} nverts: {}", dh.vb_handle, dh.pi_handle, dh.num_verts);
	    // log::debug("bind pipeline");
	    if(uint32_t hash = dh.pi_handle; hash != pipeline_hash)
	    {
	       vkCmdBindPipeline(pass.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_map[hash].pipeline);
	       // note: setting viewport and scissor when we bind pipeline incase it's changed
	       // todo: it would be nicer to have this sort of thing covered as a init frame pass or something.
	       VkViewport vp[] = {pipeline::GetDefaultViewport()};
	       vkCmdSetViewport(pass.cmd_buffer, 0, 1, vp);
	       VkRect2D scissor[] = {pipeline::GetDefaultScissor()};
	       vkCmdSetScissor(pass.cmd_buffer, 0, 1, scissor);
	       pipeline_hash = hash;
	    }
	    
	    VkBuffer vertex_buffers[] = {g_vb_map[dh.vb_handle].vb};
	    VkDeviceSize offsets[] = {0};
	 
	    // log::debug("bind vertext buffer");
	    vkCmdBindVertexBuffers(pass.cmd_buffer, 0, 1, vertex_buffers, offsets);
	 
	    // log::debug("record draw");
	    vkCmdDraw(pass.cmd_buffer, dh.num_verts, 1, 0, 0);
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
   device::CreateLogicalDevice();
   swapchain::CreateCommandPool();
   renderpass::CreateDefaultRenderPass("swapchain");
   swapchain::CreateSwapChain();
   swapchain::CreateSwapchainImageViews();
   swapchain::CreateSwapchainFrameBuffers();
   barriers::CreatePassSemaphores("swapchain");
   return 0;
}
hash::string shaders[shader::e_count];
void gGlfwVulkan::visit(class physics::collider::polygon* /*_poly*/){}
void gGlfwVulkan::visit(class camera * /*_camera*/) {}

void gGlfwVulkan::visit(fw::Mesh* _mesh)
{
   using namespace fwvulkan;
   DrawHandle drawhandle;
   //todo: don't store drawhandles against their ptr address.
   auto handles_iter = g_drawhandles.find(_mesh);
   if(handles_iter == g_drawhandles.end())
   {
      drawhandle = {
	 buffers::CreateVertexBuffer(_mesh->vbo, _mesh->vbo_len),
	 (int)_mesh->vbo_len,
	 pipeline::CreateDefaultPipeline(_mesh->mat)
      };
      g_drawhandles[_mesh] = drawhandle;
   }
   else { drawhandle = handles_iter->second; }
   for (auto pass : _mesh->passes)
   {
      g_pass_map[pass].draws.push_back(drawhandle);
   }
}
int gGlfwVulkan::shutdown()
{
   using namespace fwvulkan;
   log::scope topic("gGlfwVulkan");
   log::debug("shutdown");
   swapchain::CleanupSwapChain();
   for(auto vb : fwvulkan::g_vb_map)
   {
      vkDestroyBuffer(fwvulkan::g_logical_device, vb.second.vb, nullptr);
      vkFreeMemory(fwvulkan::g_logical_device, vb.second.vb_mem, nullptr);
   }
   for(auto pipeline : g_pipe_map)
   {
      vkDestroyPipeline(g_logical_device, pipeline.second.pipeline, nullptr);
      vkDestroyPipelineLayout(g_logical_device, pipeline.second.layout, nullptr);
   }
   g_pipe_map.clear();

   for(auto sema : g_semaphore_map)
   {
      for (int i = 0; i < fwvulkan::g_max_frames_in_flight; i++)
      {
	 vkDestroySemaphore(fwvulkan::g_logical_device, sema.second.image_available[i], nullptr);
	 vkDestroySemaphore(fwvulkan::g_logical_device, sema.second.render_finished[i], nullptr);
	 vkDestroyFence(fwvulkan::g_logical_device, sema.second.in_flight_fences[i], nullptr);
      }
   }
   vkDestroyCommandPool(fwvulkan::g_logical_device, fwvulkan::g_command_pool, nullptr);
   for(int t = fw::shader::e_fragment; t != fw::shader::e_count; t++)
   {
      for(auto s : fwvulkan::g_shaders[(fw::shader::type)t])
      {
	 vkDestroyShaderModule(fwvulkan::g_logical_device, s.second, nullptr);
      }
   }
   vkDestroyDevice(fwvulkan::g_logical_device, nullptr);
   if (fwvulkan::g_enable_validation_layers)
   {
      fwvulkan::instance::DestroyDebugUtilsMessengerEXT(fwvulkan::g_instance, fwvulkan::g_debug_messenger, nullptr);
   }
   if (fwvulkan::g_window != nullptr)
   {
      vkDestroySurfaceKHR(fwvulkan::g_instance, fwvulkan::g_surface, nullptr);
   }

   return 0;
}
int gGlfwVulkan::update() { return 0; }
int gGlfwVulkan::render()
{
   using namespace fwvulkan;
   for(auto pass : g_pass_map)
   {
      fwvulkan::buffers::RecordPass(pass.first);
   }
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
   if(renderpass::CreateDefaultRenderPass(pass))
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
