#include "wGlfwVulkan.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "../../utils/log/log.h"

namespace fwvulkan
{
   GLFWwindow* g_window = nullptr;
   bool g_resized = false;
   void FramebufferResizeCB(GLFWwindow* /*window*/, int /*width*/, int /*height*/)
   {
      g_resized = true;
   }
}

void error_callback(int code, const char* description)
{
   fw::log::debug("{}:{{}}", code, description);
    // display_error_message(code, description);
}

int wGlfwVulkan::init(int _width, int _height, const char* _name)
{
   glfwInit();
   glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwSetErrorCallback(error_callback);
   
   GLFWwindow* window = glfwCreateWindow(_width, _height, _name, nullptr, nullptr);
   if (window == nullptr)
   {
      glfwTerminate();
      return 1;
   }
   
   glfwSetWindowUserPointer(window, this);
   glfwSetFramebufferSizeCallback(window, fwvulkan::FramebufferResizeCB);
   
   m_width = _width;
   m_height = _height;
   m_name = _name;
   
   fwvulkan::g_window = window;
   
   return 0;
}
int wGlfwVulkan::update()
{
   if(fwvulkan::g_window != nullptr)
   {
      glfwPollEvents();
      return glfwWindowShouldClose(fwvulkan::g_window) == 0;
   }
   return 0;
}
int wGlfwVulkan::move(int /*_x*/, int /*_y*/)
{
  return 0;
}

int wGlfwVulkan::resize(int /*_width*/, int /*_height*/)
{
  return 0;
}

int wGlfwVulkan::shutdown()
{
   // log::debug("clean up!!");
   if (fwvulkan::g_window != nullptr)
   {
      glfwDestroyWindow(fwvulkan::g_window);
      glfwTerminate();
      fwvulkan::g_window = nullptr;
   }
   return 0;
}

window* window::windowFactory()
{
  return (window*)new wGlfwVulkan;
}
