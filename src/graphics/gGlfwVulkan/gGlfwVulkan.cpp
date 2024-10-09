#include "gGlfwVulkan.h"
#include "../../../../Libs/Vulkan/1.3.280.1/x86_64/include/vulkan/vulkan.hpp"
#include "../../../../Libs/glfw-3.4/include/GLFW/glfw3.h"
#include <cstdint>
#include <iostream>
#include <optional>
#include "../../utils/log/log.h"

using namespace fw;
namespace fwvulkan
{
   VkDebugUtilsMessengerEXT g_debug_messenger;
   VkInstance g_instance;
   VkSurfaceKHR g_surface = VK_NULL_HANDLE;
   VkPhysicalDevice g_physical_device = VK_NULL_HANDLE;
   extern GLFWwindow* g_window;
   const bool g_enable_validation_layers = false;
   const std::vector<const char *> g_device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
   const std::vector<const char *> g_validation_layers = {
      // my laptop integrated gpu has no validation layers.
      "VK_LAYER_KHRONOS_validation",
      "VK_LAYER_LUNARG_standard_validation"
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
   // physical device selection etc.
   namespace pdevice
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
	 vkEnumeratePhysicalDevices(g_instance, &device_count, devices.data());

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
	    VkPhysicalDeviceProperties device_properties;
	    vkGetPhysicalDeviceProperties(g_physical_device, &device_properties);
	    log::debug("\t{} chosen as physical device.", device_properties.deviceName);
	 }
      }
   }   
}
int gGlfwVulkan::init()
{
   log::topics::add("gGlfwVulkan");
   log::scope topic("gGlfwVulkan");
   log::debug("Init Vulkan");
   fwvulkan::instance::CreateInstance();
   if (!fwvulkan::g_enable_validation_layers)
   {
      fwvulkan::instance::SetupDebugMessenger();
   }
   if (fwvulkan::g_window != nullptr)
   {
      fwvulkan::instance::CreateSurface();
   }
   fwvulkan::pdevice::PickPhysicalDevice();
   // CreateLogicalDevice();
   // CreateSwapChain();
   // CreateImageViews();
   // CreateRenderPass();
   // CreateGraphicsPipeline();
   // CreateFrameBuffers();
   // CreateCommandPool();
   // CreateVertexBuffer();
   // CreateCommandBuffers();
   // CreateSemaphores();
   return 0;
}

void gGlfwVulkan::visit(class physics::collider::polygon* /*_poly*/){}
void gGlfwVulkan::visit(class camera* /*_camera*/){}
int gGlfwVulkan::shutdown()
{
   log::scope topic("gGlfwVulkan");
   log::debug("shutdown");
   // CleanupSwapChain();
   // vkDestroyBuffer(logical_device, vertex_buffer, nullptr);
   // vkFreeMemory(logical_device, vertex_buffer_memory, nullptr);
   // for (int i = 0; i < max_frames_in_flight; i++)
   // {
   //     vkDestroySemaphore(logical_device, image_available_semaphores[i], nullptr);
   //     vkDestroySemaphore(logical_device, render_finished_semaphores[i], nullptr);
   //     vkDestroyFence(logical_device, in_flight_fences[i], nullptr);
   // }
   // vkDestroyCommandPool(logical_device, command_pool, nullptr);
   // vkDestroyDevice(logical_device, nullptr);
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
int gGlfwVulkan::render() { return 0; }



iRenderVisitor* gGlfwVulkan::getRenderer()
{
   return (iRenderVisitor*)this;
}

graphics* graphics::graphicsFactory()
{
   return (graphics*)new gGlfwVulkan;
}
