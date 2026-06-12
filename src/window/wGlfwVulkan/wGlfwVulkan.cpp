#include "../window2.h"
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

namespace window2
{
   struct WindowOwner {} owner;
   int g_width;  //window width
   int g_height; //window height
   const char* g_name; //window name
}

void error_callback(int code, const char* description)
{
   fw::log::debug("{}:{{}}", code, description);
   // display_error_message(code, description);
}
int window2::init(int _width, int _height, const char* _name)
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

   glfwSetWindowUserPointer(window, &window2::owner);
   glfwSetFramebufferSizeCallback(window, fwvulkan::FramebufferResizeCB);

   window2::g_width = _width;
   window2::g_height = _height;
   window2::g_name = _name;

   fwvulkan::g_window = window;

   return 0;
}
int window2::update()
{
   if (fwvulkan::g_window != nullptr)
   {
      glfwPollEvents();
      return glfwWindowShouldClose(fwvulkan::g_window) == 0;
   }
   return 0;
}

int window2::shutdown()
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