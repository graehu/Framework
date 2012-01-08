#include "wSdl.h"

int wSdl::init(int _width, int _height, char* _name)
{

  //checks if sdl init video is sucessful
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
    printf("Unable to initialize SDL: %s\n", SDL_GetError());

  atexit(SDL_Quit);

  m_screen = SDL_SetVideoMode(
			     _width, 
			     _height, 
			     16, 
			     SDL_DOUBLEBUF | SDL_HWPALETTE
			     ); 
  //| SDL_FULLSCREEN | SDL_DOUBLEBUF

  if (m_screen == NULL)
    printf("Unable to set video mode: %s\n", SDL_GetError());

  SDL_WM_SetCaption((const char*)_name, NULL); // remember to set the icon at some point in the future.

  m_width = _width;
  m_height = _height;
  m_name = _name;
  return 0;
}

int wSdl::getWidth()
{
  return m_width;
}

int wSdl::getHeight()
{
  return m_height;
}

int wSdl::move(int _x, int _y)
{
  return 0;
}

int wSdl::resize(int _width, int _height)
{
  return 0;
}


window* window::windowFactory()
{
  return (window*)new wSdl;
}
