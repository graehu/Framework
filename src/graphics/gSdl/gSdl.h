#ifndef GSDL_H
#define GSDL_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
//#include <SDL/SDL_draw.h>
#include <map>


#include "../graphics.h"
#include "../renderable/sprite/sprite.h"


class gSdl : public graphics, public iRenderVisitor
{
 public:

  gSdl(){}
  ~gSdl(){}

  //graphics section

  int update();
  int render();
  int init();
  int shutdown();
  
  iRenderVisitor* getRenderer(void);

  //visitor section(renderer)
  void visit(sprite* _sprite);

 protected:

	int loadImage(char* _fileName);
	int unloadImage(int _imageID);


	 //std::vector<std::pair<SDL_Surface, char*> > m_images; //the surface, then the filename
	 std::map<char*,SDL_Surface> m_images;

  SDL_Surface* m_windowSurface;

 private:

};
#endif//GSDL_H
