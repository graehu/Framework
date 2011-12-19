#ifndef GSDL_H
#define GSDL_H

#include <SDL/SDL.h>
#include <vector>
#include "../graphics.h"

class gSdl : public graphics
{
 public:

  gSdl(){}
  ~gSdl(){}

  int init();
  int update();
  int render();
  int loadImage(char* _fileName);
  int blitImage(int _imageID, rect _source, rect _destination);
  int unloadImage(int _imageID);
  int shutdown();


 protected:

  std::vector<SDL_Surface> m_images;

  SDL_Surface* gBackBuffer;
  SDL_Surface* m_windowSurface;

 private:

};
#endif//GSDL_H
