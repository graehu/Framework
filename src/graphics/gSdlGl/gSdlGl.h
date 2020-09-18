#ifndef GSDLGL_H
#define GSDLGL_H

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <vector>
#include <map>

#include "../graphics.h"
#include "../../types/mat4x4f.h"

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
   void visit(class physics::collider::polygon* _poly);
  void visit(class camera* _camera);

 protected:

	 //void loadModelTextures(model* _model);
	 void loadImage(char* _fileName);
   void loadOptimisedImage(char* _fileName);

	 std::map<char*,std::pair<SDL_Surface, GLuint> > m_images;

  //needed for cameras
  mat4x4f m_projMat; // projectionMatrix; // Store the projection matrix
  mat4x4f m_viewMat; // viewMatrix; // Store the view matrix
  mat4x4f m_modelMat; // modelMatrix; // Store the model matrix

  SDL_Surface* m_windowSurface;

 private:

};
#endif//GSDLGL_H
