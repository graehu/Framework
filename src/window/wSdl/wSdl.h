#ifndef WSDL_H
#define WSDL_H

#include <SDL/SDL.h>
#include "../window.h"

class wSdl : public window
{
 public:

  int init(int _width, int _height, char* _name);
  int move(int _x, int _y);
  int resize(int _width, int _height);
  int getHeight(void);
  int getWidth(void);
  

 protected:

  SDL_Surface* m_screen;

 private:
};

#endif /*WSDL_H*/
