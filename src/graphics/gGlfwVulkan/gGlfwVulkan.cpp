#include "gGlfwVulkan.h"
#include "../../../../Libs/Vulkan/1.3.280.1/x86_64/include/vulkan/vulkan.hpp"
#include "../../../../Libs/glfw-3.4/include/GLFW/glfw3.h"
#include <cstdint>
#include <iostream>
#include "../../utils/log/log.h"

using namespace fw;
bool gGlfwVulkan::CheckValidationLayerSupport()
{
   uint32_t layer_count = 0;
   uint32_t layers_found = 0;
   vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
   std::vector<VkLayerProperties> available_layers(layer_count);
   vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
   for (const char *layer_name : m_validation_layers)
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
   return layers_found == m_validation_layers.size();
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

VkInstance g_instance;
void gGlfwVulkan::CreateInstance()
{
   if (m_enable_validation_layers && !CheckValidationLayerSupport())
   {
      log::fatal("fatal, unable to enable valiadation layers");
   }
   // app info
   VkApplicationInfo app_info = {};
   app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   app_info.pApplicationName = "Vulkanism";
   app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
   app_info.pEngineName = "No engine";
   app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
   app_info.apiVersion = VK_API_VERSION_1_3;
   // create info
   VkInstanceCreateInfo create_info = {};
   create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   create_info.pApplicationInfo = &app_info;

   VkDebugUtilsMessengerCreateInfoEXT create_debug_info = {};
   if (m_enable_validation_layers)
   {
      create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
      create_info.ppEnabledLayerNames = m_validation_layers.data();
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
   if (m_enable_validation_layers)
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
   m_instance = g_instance;
}

int gGlfwVulkan::init()
{
   log::topics::add("gGlfwVulkan");
   log::scope topic("gGlfwVulkan");
   log::debug("Init Vulkan");
   CreateInstance();
   // SetupDebugMessenger();
   // if (window != nullptr)
   // {
   //     CreateSurface();
   // }
   // PickPhysicalDevice();
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
int gGlfwVulkan::shutdown() { return 0; }
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