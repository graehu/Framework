#include "wGlfwVulkan.h"
#define GLFW_INCLUDE_VULKAN
#include "../../../../Libs/glfw-3.4/include/GLFW/glfw3.h"

namespace fwvulkan
{
   GLFWwindow* g_window = nullptr;
   bool g_resized = false;
   void FramebufferResizeCB(GLFWwindow* /*window*/, int /*width*/, int /*height*/)
   {
      g_resized = true;
   }
}

int wGlfwVulkan::init(int _width, int _height, const char* _name)
{
   glfwInit();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
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
