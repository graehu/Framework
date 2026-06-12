#pragma once

namespace window2
{
   int init(int _width, int _height, const char* _name);
   int update();
   int move(int _x, int _y);
   int resize(int _width, int _height);
   int shutdown();
   int get_height();
   int get_width();
};
