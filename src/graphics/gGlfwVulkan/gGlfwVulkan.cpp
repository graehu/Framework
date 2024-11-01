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
   VkFormat g_swap_chain_image_format;
   VkExtent2D g_swap_chain_extent;
   std::vector<VkImage> g_swap_chain_images;
   std::vector<VkImageView> g_swap_chain_image_views;
   std::vector<VkFramebuffer> g_swap_chain_framebuffers;
   // semaphores
   std::vector<VkSemaphore> g_image_available_semaphores;
   std::vector<VkSemaphore> g_render_finished_semaphores;
   std::vector<VkFence> g_in_flight_fences;
   const int g_max_frames_in_flight = 2;
   unsigned int g_current_frame = 0;
   unsigned int g_flight_frame = 0;
   unsigned int g_current_buffers = 0;
   // renderpass
   // the default one used for the swapchain
   struct PassHandler
   {
      // PassHandler(VkCommandBuffer c, VkRenderPass p) : cmd_buffer(c), pass(p){}
      VkRenderPass pass = VK_NULL_HANDLE;
      VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
   };
   std::map<fw::hash::string, PassHandler> g_pass_map;
   
   // shaders
   typedef std::map<fw::hash::string, VkShaderModule> shader_map;
   std::array<shader_map, fw::shader::e_count> g_shaders;
   // pipeline
   std::vector<VkPipeline> g_pipelines;
   std::vector<VkPipelineLayout> g_pipeline_layouts;
   // commands
   VkCommandPool g_command_pool = VK_NULL_HANDLE;
   std::vector<VkCommandBuffer> g_command_buffers;
   // vertex buffers
   std::vector<VkBuffer> g_vertex_buffers;
   std::vector<VkDeviceMemory> g_vertex_buffer_memory;
   // mesh
   std::vector<Mesh*> g_meshes;
   struct drawhandles
   {
      int vb_handle = -1;
      int num_verts = 0;
      int pi_handle = -1;
      // int cb_handle = -1; // these should go with renderpasses
   };
   std::map<fw::Mesh *, drawhandles> g_drawhandles;
   //
   bool g_enable_validation_layers = false;
   const std::vector<const char*> g_instance_extensions = {
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
   };
   const std::vector<const char *> g_device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_MEMORY_BUDGET_EXTENSION_NAME
   };
   // Note: These layers require you to run Vulkan/1.3.280.1/setup-env.sh prior to running the executable.
   const std::vector<const char *> g_validation_layers = {
       "VK_LAYER_KHRONOS_validation"
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
	 if (g_enable_validation_layers && !CheckValidationLayerSupport())
	 {
	    log::fatal("fatal, unable to enable valiadation layers");
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
	 }
	 for(auto ext : g_instance_extensions)
	 {
	    required_extensions.push_back(ext);
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
	 // VK_KHR_wayland_surface is supported, consider using that.
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
	 assert(g_swap_chain_framebuffers.size() == 0);
	 g_swap_chain_framebuffers.resize(g_swap_chain_image_views.size());
	 for (size_t i = 0; i < g_swap_chain_image_views.size(); i++)
	 {
	    VkImageView attachments[] = {g_swap_chain_image_views[i]};
	    VkFramebufferCreateInfo framebuffer_create_info = {};
	    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	    framebuffer_create_info.renderPass = g_pass_map["default"].pass;
	    framebuffer_create_info.attachmentCount = 1;
	    framebuffer_create_info.pAttachments = attachments;
	    framebuffer_create_info.width = g_swap_chain_extent.width;
	    framebuffer_create_info.height = g_swap_chain_extent.height;
	    framebuffer_create_info.layers = 1;

	    if (vkCreateFramebuffer(g_logical_device, &framebuffer_create_info, nullptr, &g_swap_chain_framebuffers[i]) !=
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
	 vkFreeCommandBuffers(g_logical_device, g_command_pool, static_cast<uint32_t>(g_command_buffers.size()),
			      g_command_buffers.data());
	 g_command_buffers.clear();
	 
	 for (auto framebuffer : g_swap_chain_framebuffers)
	 {
	    vkDestroyFramebuffer(g_logical_device, framebuffer, nullptr);
	 }
	 g_swap_chain_framebuffers.clear();

	 for(auto pipeline : g_pipelines)
	 {
	    vkDestroyPipeline(g_logical_device, pipeline, nullptr);
	 }
	 g_pipelines.clear();
	 
	 for(auto layout : g_pipeline_layouts)
	 {
	    vkDestroyPipelineLayout(g_logical_device, layout, nullptr);
	 }
	 g_pipeline_layouts.clear();
	 
	 for (auto image_view : g_swap_chain_image_views)
	 {
	    vkDestroyImageView(g_logical_device, image_view, nullptr);
	 }
	 g_swap_chain_image_views.clear();
	 for (auto pass : g_pass_map)
	 {
	    vkDestroyRenderPass(g_logical_device, pass.second.pass, nullptr);
	 }
	 g_pass_map.clear();
	 vkDestroySwapchainKHR(g_logical_device, g_swap_chain, nullptr);
	 g_swap_chain = VK_NULL_HANDLE;
	 g_drawhandles.clear();
      }
      void CreateSwapchainImageViews()
      {
	 log::debug("Create Swapchain Image Views");
	 assert(g_swap_chain_image_views.size() == 0);
	 g_swap_chain_image_views.resize(g_swap_chain_images.size());

	 for (size_t i = 0; i < g_swap_chain_images.size(); i++)
	 {
	    VkImageViewCreateInfo create_info = {};
	    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	    create_info.image = g_swap_chain_images[i];
	    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	    create_info.format = g_swap_chain_image_format;

	    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	    create_info.subresourceRange.baseMipLevel = 0;
	    create_info.subresourceRange.levelCount = 1;
	    create_info.subresourceRange.baseArrayLayer = 0;
	    create_info.subresourceRange.layerCount = 1;

	    if (vkCreateImageView(g_logical_device, &create_info, nullptr, &g_swap_chain_image_views[i]) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create image viaews!");
	    }
	 }
      }
      VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> available_formats)
      {
	 for (const auto &available_format : available_formats)
	 {
	    if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
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
	 g_swap_chain_images.resize(image_count);
	 vkGetSwapchainImagesKHR(g_logical_device, g_swap_chain, &image_count, g_swap_chain_images.data());

	 g_swap_chain_image_format = surface_format.format;
         g_swap_chain_extent = extent;
      }
   }
   namespace renderpass
   {
      void CreateDefaultRenderPass(hash::string passname)
      {
	 log::debug("CreateDefaultRenderPass");
	 if(g_pass_map.find(passname) == g_pass_map.end())
	 {
	    VkAttachmentDescription color_attachement = {};
	    color_attachement.format = g_swap_chain_image_format;
	    color_attachement.samples = VK_SAMPLE_COUNT_1_BIT;
	    color_attachement.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	    color_attachement.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	    color_attachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	    color_attachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	    color_attachement.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	    color_attachement.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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
	    g_pass_map[passname] = {pass, buffer};
	    g_command_buffers.push_back(buffer);
	    log::debug("Renderpass '{}' created.", passname.m_literal);

	 }
      }
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
	 CreateSwapchainImageViews();
	 renderpass::CreateDefaultRenderPass("default");
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
      VkPipelineViewportStateCreateInfo GetDefaultViewportState()
      {
	 // note: the statics are because they're passed by ref below.
	 // ----: similar should be done if a non default viewport state func is made.
	 static VkViewport viewport = {};
	 viewport.x = 0.0f;
	 viewport.y = 0.0f;
	 viewport.width = (float)g_swap_chain_extent.width;
	 viewport.height = (float)g_swap_chain_extent.height;
	 viewport.minDepth = 0.0f;
	 viewport.maxDepth = 1.0f;

	 static VkRect2D scissor = {};
	 scissor.offset = {0, 0};
	 scissor.extent = g_swap_chain_extent;

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
      int CreatePipeline(Material mat)
      {
	 // todo: rename create default pipeline, or supply args to control below defaults.
	 // todo: add a cache inside here which can return handles instead of creating new pipelines.
	 log::debug("CreateGraphicsPipeline");
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
	 pipeline_ci.renderPass = g_pass_map["default"].pass;
	 pipeline_ci.subpass = 0;
	 pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
	 pipeline_ci.basePipelineIndex = -1;
	 
	 VkPipeline graphics_pipeline;
	 if (vkCreateGraphicsPipelines(g_logical_device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr,
				       &graphics_pipeline) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to create graphics pipeline!");
	 }

	 g_pipelines.push_back(graphics_pipeline);
	 g_pipeline_layouts.push_back(pipeline_layout);
	 log::debug("pipeline created id: {}", g_pipelines.size()-1);
	 return g_pipelines.size() - 1;
      }
   }
   namespace buffers
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

      int CreateVertexBuffer(const fw::Vertex* vertices, int num_vertices)
      {
	 // todo: make a cache? maybe?
	 log::debug("Create Vertex Buffer");
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
	 allocate_info.memoryTypeIndex = FindMemoryType(
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
	 g_vertex_buffers.push_back(vertex_buffer);
	 g_vertex_buffer_memory.push_back(vertex_buffer_memory);
	 log::debug("Created Vertex Buffer: {}", g_vertex_buffers.size()-1);
	 return g_vertex_buffers.size()-1;
      }
      
      int CreateCommandBuffer()
      {
	 log::debug("Create Command buffer");
	 VkCommandBufferAllocateInfo alloc_info = {};
	 alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	 alloc_info.commandPool = g_command_pool;
	 alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	 alloc_info.commandBufferCount = 1;// (uint32_t)command_buffers.size();
	 VkCommandBuffer buffer;
	 if (vkAllocateCommandBuffers(g_logical_device, &alloc_info, &buffer) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to allocate command buffers!");
	 }
	 g_command_buffers.push_back(buffer);
	 log::debug("Created command buffer: {}", g_command_buffers.size()-1);
	 return g_command_buffers.size()-1;
      }
      void RecordPass(hash::string pass, std::vector<drawhandles> dhs)
      {
	 VkCommandBufferBeginInfo begin_info = {};
	 begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	 begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	 begin_info.pInheritanceInfo = nullptr;
	 
	 auto cb = g_pass_map[pass].cmd_buffer;
	 if (vkBeginCommandBuffer(cb, &begin_info) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to begin recording command buffer!");
	 }
	 VkRenderPassBeginInfo render_pass_begin_info = {};
	 render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	 render_pass_begin_info.renderPass = g_pass_map["default"].pass;
	 render_pass_begin_info.framebuffer = g_swap_chain_framebuffers[g_current_buffers];
	 render_pass_begin_info.renderArea.offset = {0, 0};
	 render_pass_begin_info.renderArea.extent = g_swap_chain_extent;

	 // log::debug("bound fb: {}", g_current_frame);

	 VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}}; // fixes a warning
	 render_pass_begin_info.clearValueCount = 1;
	 render_pass_begin_info.pClearValues = &clear_color;
	 
	 // log::debug("begin renderpass");
	 vkCmdBeginRenderPass(cb, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	 for(auto dh : dhs)
	 {
	    // log::debug("Recording Draw vb: {} pi: {} nverts: {}", dh.vb_handle, dh.pi_handle, dh.num_verts);
	    // log::debug("bind pipeline");
	    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipelines[dh.pi_handle]);


	    VkBuffer vertex_buffers[] = {g_vertex_buffers[dh.vb_handle]};
	    VkDeviceSize offsets[] = {0};
	 
	    // log::debug("bind vertext buffer");
	    vkCmdBindVertexBuffers(cb, 0, 1, vertex_buffers, offsets);
	 
	    // log::debug("record draw");
	    vkCmdDraw(cb, dh.num_verts, 1, 0, 0);
	 }
	 // log::debug("end renderpass");
	 vkCmdEndRenderPass(cb);
	 
	 // log::debug("end commandbuffer");
	 if (vkEndCommandBuffer(cb) != VK_SUCCESS)
	 {
	    throw std::runtime_error("failed to record command buffer!");
	 }
      }
   }
   namespace barriers
   {
      void CreateSemaphores()
      {
	 log::debug("CreateSemaphores");
	 assert(g_image_available_semaphores.size() == 0);
	 assert(g_render_finished_semaphores.size() == 0);
	 assert(g_in_flight_fences.size() == 0);
	 
	 g_image_available_semaphores.resize(g_max_frames_in_flight);
	 g_render_finished_semaphores.resize(g_max_frames_in_flight);
	 g_in_flight_fences.resize(g_max_frames_in_flight);

	 VkSemaphoreCreateInfo semaphore_create_info = {};
	 semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	 VkFenceCreateInfo fence_create_info = {};
	 fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	 fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	 for (int i = 0; i < g_max_frames_in_flight; i++)
	 {
	    if (vkCreateSemaphore(g_logical_device, &semaphore_create_info, nullptr, &g_image_available_semaphores[i]) !=
		VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create semaphore!");
	    }

	    if (vkCreateSemaphore(g_logical_device, &semaphore_create_info, nullptr, &g_render_finished_semaphores[i]) !=
		VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create semaphore!");
	    }

	    if (vkCreateFence(g_logical_device, &fence_create_info, nullptr, &g_in_flight_fences[i]) != VK_SUCCESS)
	    {
	       throw std::runtime_error("failed to create semaphore!");
	    }
	 }
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
   swapchain::CreateSwapChain();
   swapchain::CreateSwapchainImageViews();
   swapchain::CreateCommandPool();
   renderpass::CreateDefaultRenderPass("default"); // user to do, except for swapchain / default renderpass
   // pipeline::CreateGraphicsPipeline(); // user to do.
   swapchain::CreateSwapchainFrameBuffers();
   // buffers::CreateVertexBuffer(); // user to do
   // buffers::CreateCommandBuffers(); // user to do, except for swapchain / default renderpass
   barriers::CreateSemaphores();   // user to do, except for swapchain / default renderpass
   return 0;
}
hash::string shaders[shader::e_count];
void gGlfwVulkan::visit(class physics::collider::polygon* /*_poly*/){}
void gGlfwVulkan::visit(class camera * /*_camera*/) {}
std::vector<fwvulkan::drawhandles> g_draws;
void gGlfwVulkan::visit(fw::Mesh* _mesh)
{
   using namespace fwvulkan;
   drawhandles drawhandle;

   auto handles_iter = g_drawhandles.find(_mesh);
   if(handles_iter == g_drawhandles.end())
   {
      drawhandle = {
	 buffers::CreateVertexBuffer(_mesh->vbo, _mesh->vbo_len),
	 (int)_mesh->vbo_len,
	 pipeline::CreatePipeline(_mesh->mat)
      };
      g_drawhandles[_mesh] = drawhandle;
   }
   else { drawhandle = handles_iter->second; }
   g_draws.push_back(drawhandle);
}
int gGlfwVulkan::shutdown()
{
   using namespace fwvulkan;
   log::scope topic("gGlfwVulkan");
   log::debug("shutdown");
   swapchain::CleanupSwapChain();
   for(auto vb : fwvulkan::g_vertex_buffers)
   {
      vkDestroyBuffer(fwvulkan::g_logical_device, vb, nullptr);
   }
   for(auto vbm : fwvulkan::g_vertex_buffer_memory)
   {
      vkFreeMemory(fwvulkan::g_logical_device, vbm, nullptr);      
   }

   for (int i = 0; i < fwvulkan::g_max_frames_in_flight; i++)
   {
       vkDestroySemaphore(fwvulkan::g_logical_device, fwvulkan::g_image_available_semaphores[i], nullptr);
       vkDestroySemaphore(fwvulkan::g_logical_device, fwvulkan::g_render_finished_semaphores[i], nullptr);
       vkDestroyFence(fwvulkan::g_logical_device, fwvulkan::g_in_flight_fences[i], nullptr);
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

   // the window should clean these up.
   // vkDestroyInstance(instance, nullptr);
   // if (window != nullptr)
   // {
   //     glfwDestroyWindow(window);
   //     glfwTerminate();
   // }
   return 0;
}
int gGlfwVulkan::update() { return 0; }
int gGlfwVulkan::render()
{
   using namespace fwvulkan;
   fwvulkan::buffers::RecordPass("default", g_draws);
   // log::debug("render frames: {} {} {}", g_current_frame, g_flight_frame, g_current_buffers);
   vkWaitForFences(g_logical_device, 1, &g_in_flight_fences[g_flight_frame], VK_TRUE, std::numeric_limits<uint64_t>::max());

   uint32_t image_index;
   VkResult result = vkAcquireNextImageKHR(g_logical_device, g_swap_chain, std::numeric_limits<uint64_t>::max(),
					   g_image_available_semaphores[g_flight_frame], VK_NULL_HANDLE, &image_index);
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
   VkSubmitInfo submit_info = {};
   submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

   VkSemaphore wait_semaphores[] = {g_image_available_semaphores[g_flight_frame]};
   VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
   submit_info.waitSemaphoreCount = 1;
   submit_info.pWaitSemaphores = wait_semaphores;
   submit_info.pWaitDstStageMask = wait_stages;
   submit_info.commandBufferCount = g_command_buffers.size();
   // log::debug("image index: {}", image_index);
   submit_info.pCommandBuffers = g_command_buffers.data();
   // log::debug("num command buffers: {}", g_command_buffers.size());

   VkSemaphore signal_semaphores[] = {g_render_finished_semaphores[g_flight_frame]};
   submit_info.signalSemaphoreCount = 1;
   submit_info.pSignalSemaphores = signal_semaphores;

   vkResetFences(g_logical_device, 1, &g_in_flight_fences[g_flight_frame]);
   if (vkQueueSubmit(g_graphics_queue, 1, &submit_info, g_in_flight_fences[g_flight_frame]) != VK_SUCCESS)
   {
      throw std::runtime_error("failed to submit draw command buffer!");
   }
   
   VkPresentInfoKHR present_info = {};
   present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   present_info.waitSemaphoreCount = 1;
   present_info.pWaitSemaphores = signal_semaphores;
   VkSwapchainKHR swap_chains[] = {g_swap_chain};
   present_info.swapchainCount = 1;
   present_info.pSwapchains = swap_chains;
   present_info.pImageIndices = &image_index;
   present_info.pResults = nullptr;

   result = vkQueuePresentKHR(g_present_queue, &present_info);
   if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
   {
      swapchain::RecreateSwapChain();
   }
   else if (result != VK_SUCCESS)
   {
      throw std::runtime_error("failed to present swap chain image!");
   }
   g_current_frame++;
   g_flight_frame = g_current_buffers % g_max_frames_in_flight;
   g_current_buffers = g_current_frame % g_swap_chain_framebuffers.size();
   vkResetCommandPool(g_logical_device, g_command_pool, 0);
   g_draws.clear();
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

bool gGlfwVulkan::register_pass(fw::hash::string)
{
   using namespace fwvulkan;
   return true;
}

iRenderVisitor* gGlfwVulkan::getRenderer()
{
   return (iRenderVisitor*)this;
}

graphics* graphics::graphicsFactory()
{
   return (graphics*)new gGlfwVulkan;
}
