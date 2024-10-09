#ifndef WWINGL4_H
#define WWINGL4_H

#include "../window.h"

class wWinGl4 : public window
{
public:
  int init(int _width, int _height, char* _name);
  int move(int _x, int _y);
  int resize(int _width, int _height);
   int shutdown() {return 0;}

private:
protected:


};
#endif//WWINGL4_H
