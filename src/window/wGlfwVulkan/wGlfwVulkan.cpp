#include "wGlfwVulkan.h"
#define GLFW_INCLUDE_VULKAN
#include "../../../../Libs/glfw-3.4/include/GLFW/glfw3.h"

void wGlfwVulkan::FramebufferResizeCB(GLFWwindow* window, int /*width*/, int /*height*/)
{
    wGlfwVulkan* self = reinterpret_cast<wGlfwVulkan*>(glfwGetWindowUserPointer(window));
    self->m_resized = true;
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
   glfwSetFramebufferSizeCallback(window, FramebufferResizeCB);
   
   m_width = _width;
   m_height = _height;
   m_name = _name;
   
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


window* window::windowFactory()
{
  return (window*)new wGlfwVulkan;
}
