#pragma once

#include "../window.h"

class wGlfwVulkan : public window
{
 public:

  int init(int _width, int _height, const char* _name);
  int move(int _x, int _y);
  int resize(int _width, int _height);
   int shutdown();

};
