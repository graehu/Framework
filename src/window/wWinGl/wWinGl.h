#ifndef WWINGL_H
#define WWINGL_H

#include "../window.h"

class wWinGl : public window
{
public:
	wWinGl(){};
	~wWinGl(){};

  int init(int _width, int _height, char* _name);
  int move(int _x, int _y);
  int resize(int _width, int _height);

private:
protected:


};
#endif//WWINGL_H
