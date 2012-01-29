#ifndef WSDLGL_H
#define WSDLGL_H

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <GL/glu.h>
#include <GL/gl.h>

#include "../window.h"

class wSdlGl : public window
{
 public:

  int init(int _width, int _height, char* _name);
  int move(int _x, int _y);
  int resize(int _width, int _height);
  
 protected:

  SDL_Surface* m_screen;

 private:
};

#endif//WSDLGL_H
