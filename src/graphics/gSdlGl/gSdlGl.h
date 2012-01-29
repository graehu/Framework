#ifndef GSDLGL_H
#define GSDLGL_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_opengl.h>
#include <GL/gl.h>
#include <vector>


#include "../graphics.h"

#include "../renderable/sprite/sprite.h"
#include "../renderable/3DObject/object3D.h"


#include "../resources/model/model.h"


class gSdlGl : public graphics, public iRenderVisitor
{
 public:

  gSdlGl(){}
  ~gSdlGl(){}

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
  void visit(object3D* _object3D);
  void visit(bezierCurve* _bezierCurve){printf("render bezierCurve\n");}

 protected:

	 void loadModelTextures(model* _model);

  std::vector<SDL_Surface> m_images;
  std::vector<model*> m_models;

  SDL_Surface* m_windowSurface;

 private:

};
#endif//GSDLGL_H
