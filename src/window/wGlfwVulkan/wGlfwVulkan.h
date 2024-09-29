#pragma once

#include "../window.h"
struct GLFWwindow;
class wGlfwVulkan : public window
{
 public:

  int init(int _width, int _height, const char* _name);
  int move(int _x, int _y);
  int resize(int _width, int _height);
  
 protected:
   static void FramebufferResizeCB(GLFWwindow *window, int width, int height);
   GLFWwindow* m_window;
   bool m_resized;

};
