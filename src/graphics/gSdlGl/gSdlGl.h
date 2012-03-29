#ifndef GSDLGL_H
#define GSDLGL_H

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <vector>
#include <map>

#include "../graphics.h"
#include "../renderable/sprite/sprite.h"

//#include "../resources/model/model.h"


class gSdlGl : public graphics, public iRenderVisitor
{
 public:

  gSdlGl(){}
  ~gSdlGl(){}

  //graphics section

  int update();
  int render();


  int init();
  int shutdown();
  
  iRenderVisitor* getRenderer(void);

  //visitor section

  void visit(sprite* _sprite);


 protected:

	 //void loadModelTextures(model* _model);
	 void loadImage(char* _fileName);
     void loadOptimisedImage(char* _fileName);

	 std::map<char*,std::pair<SDL_Surface, GLuint> > m_images;
  //std::vector<model*> m_models;

  SDL_Surface* m_windowSurface;

 private:

};
#endif//GSDLGL_H
