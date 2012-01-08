#ifndef GSDL_H
#define GSDL_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <vector>


#include "../graphics.h"
#include "../renderable/sprite/sprite.h"
#include "../renderable/sprite/animSprite/animSprite.h"


class gSdl : public graphics, public iRenderVisitor
{
 public:

  gSdl(){}
  ~gSdl(){}

  //graphics section

  int update();
  int render();

  int loadImage(char* _fileName);
  int unloadImage(int _imageID);

  int init();
  int shutdown();
  
  iRenderVisitor* getRenderer(void);

  //visitor section

  void visit(sprite* _sprite);
  void visit(animSprite* _animSprite){printf("odd\n");}

 protected:

  std::vector<SDL_Surface> m_images;
  SDL_Surface* m_windowSurface;

 private:

};
#endif//GSDL_H
