#pragma once

#include "../window.h"

class wGlfwVulkan : public window
{
public:

   int init(int _width, int _height, const char* _name) override;
   int move(int _x, int _y) override;
   int update() override;
   int resize(int _width, int _height) override;
   int shutdown() override;
};
