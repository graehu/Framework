#include "gSdlGl.h"
#include <cassert>
//graphics libs
#include <SDL/SDL_image.h>
#include <SDL/SDL_opengl.h>
#include "../../types/mat4x4f.h"


#pragma comment(lib, "SDL_image.lib")
//#pragma comment(lib, "opengl32.lib")

// This file shouldn't have any rendering in it. All of the utilities go in here like image loading
// asset clean up, shader loading, yada yada.


GLuint textureGen(SDL_Surface* _surface)
{
   GLuint texture;			// This is a handle to our texture object
   GLenum texture_format;
   GLint  nOfColors;
	 
   // get the number of channels in the SDL surface
   nOfColors = _surface->format->BytesPerPixel;

   if (nOfColors == 4)     // contains an alpha channel
   {
      if (_surface->format->Rmask == 0x000000ff)
	 texture_format = GL_RGBA;
      else
	 texture_format = GL_BGRA;
   } 
   else if (nOfColors == 3)     // no alpha channel
   {
      if (_surface->format->Rmask == 0x000000ff)
	 texture_format = GL_RGB;
      else
	 texture_format = GL_BGR;
   } 
   else 
   {
      printf("warning: the image is not truecolor..  this will probably break\n");
      texture_format = GL_BGR;
      texture_format = GL_RGB;
      // this error should not go unhandled
   }

   // Have OpenGL generate a texture object handle for us
   glGenTextures(1, &texture);

   // Bind the texture object
   glBindTexture( GL_TEXTURE_2D, texture );

   // Set the texture's stretching properties
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

   // Edit the texture object's image data using the information SDL_Surface gives us
   glTexImage2D	(
      GL_TEXTURE_2D, 
      0, 
      nOfColors, 
      _surface->w, 
      _surface->h, 
      0,
      texture_format, 
      GL_UNSIGNED_BYTE, 
      _surface->pixels
      );

   return texture;
}



int gSdlGl::init()
{
   m_windowSurface = SDL_GetVideoSurface();
   SDL_SetColorKey(m_windowSurface, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(m_windowSurface->format, 0xff, 0x00, 0xff));
   return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int gSdlGl::update()
{
   return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//UTILITIES
void gSdlGl::loadImage(char* _fileName)
{
   if(m_images.find(_fileName) != m_images.end())
      return;

   SDL_Surface* l_loadedImage;
   l_loadedImage = IMG_Load(_fileName);

   if (l_loadedImage == NULL)
   {
      printf("Unable to load image: %s\n", SDL_GetError());
      assert(false);
      return;
   }
   else
   {
      GLuint texture = textureGen(l_loadedImage);
      m_images[_fileName] = std::pair<SDL_Surface, GLuint>(*l_loadedImage,texture);
   }
   return;
}

void gSdlGl::loadOptimisedImage(char* _fileName)
{
   if(m_images.find(_fileName) != m_images.end())
      return;
   loadImage(_fileName);
   SDL_Surface* l_optimisedImage = SDL_DisplayFormat(&m_images[_fileName].first);
   if(l_optimisedImage == NULL)
   {
      printf("Unable to load image: %s\n", SDL_GetError());
      assert(false);
      return;
   }
   else
   {
      Uint32 colourkey = SDL_MapRGB(l_optimisedImage->format, 0xFF, 0, 0xFF );
      SDL_SetColorKey(l_optimisedImage, SDL_SRCCOLORKEY, colourkey);
      m_images[_fileName].first = *l_optimisedImage;
      SDL_FreeSurface(l_optimisedImage);
   }
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




int gSdlGl::render()
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glMultMatrixf(&m_projMat.elem[0][0]);

   glMatrixMode(GL_MODELVIEW);// Select The Modelview Matrix
   glLoadIdentity();// Reset The Modelview Matrix

   mat4x4f m_model_view = m_viewMat*m_modelMat;
   glMultMatrixf(&m_model_view.elem[0][0]);

   SDL_GL_SwapBuffers();
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear The Screen And The Depth Buffer
   //
   return 0;
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




int gSdlGl::shutdown()
{
   //delete any and all resources before ending the program with this function

   //might want to think about a way to delete all the gObjects that could have been created by this point.
   //not sure if its actually nessisary to be honest

   return 0;

} //shutsdown the graphics engine, probably called in the destructor or something


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

iRenderVisitor* gSdlGl::getRenderer()
{
   return (iRenderVisitor*)this;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

graphics* graphics::graphicsFactory()
{
   return (graphics*)new gSdlGl;
}




















// a bunch of 3d calls.
/*
  void drawSphere(vec3f _colour, vec3f _position)
  {
  glPushMatrix();
  glColor3f(_colour.i,_colour.j,_colour.k);
  GLUquadricObj *quadric;
  quadric = gluNewQuadric();
  gluQuadricDrawStyle(quadric, GLU_FILL );
  glTranslatef(_position.i, _position.j, _position.k);
  gluSphere(quadric,0.2,8,8);
  glLoadIdentity();
  gluDeleteQuadric(quadric); 
  glColor3f(1.0,1.0,1.0);
  glPopMatrix();
  }

  void gSdlGl::visit(coasterRider* _coasterRider)
  {
  drawSphere(vec3f(1,1,1),_coasterRider->getPos());
  }

  void gSdlGl::visit(bezierCurve* _bezierCurve)
  {

  if(_bezierCurve->getNumPoints() > 0)
  {
  if(_bezierCurve->getSelectedID() != 0)
  {
  drawSphere(vec3f(0.0,0.0,1.0), _bezierCurve->getPoint(0));
  }

  vec3f p;
  float mu;
  mu = 0.0f;
  glLineWidth(2);
  glBegin(GL_LINE_STRIP);
  for(int i = 0;  i < (_bezierCurve->getNumPoints()-1)/3; i++)
  {
  glBegin(GL_LINE_STRIP);
  glColor3f(0,0,0);
  for(int ii = 0; ii <= 20; ii++) //20 = quality
  {
  p = _bezierCurve->getPointOnCurve(i, mu);
  mu += 0.05f;
  glVertex3d(p.i, p.j, p.k);
  }
  glColor3f(1,1,1);
  glEnd();
  glLineWidth(1);
  mu = 0.0f;
  if((i+1)*3 != _bezierCurve->getSelectedID())
  drawSphere(vec3f(0.0,0.0,1.0), p);
  }
  if(_bezierCurve->getSelectedID() != -1)
  {
  if((_bezierCurve->getSelectedID()%3) == 0)
  {

  if(_bezierCurve->getSelectedID() == 0)
  {
  drawSphere(vec3f(1.0,0.0,0.0), _bezierCurve->getPoint(_bezierCurve->getSelectedID()));
  drawSphere(vec3f(0.0,1.0,0.0), _bezierCurve->getPoint(_bezierCurve->getSelectedID()+1));
  }
  else if(_bezierCurve->getSelectedID() == _bezierCurve->getNumPoints()-1)
  {
  drawSphere(vec3f(1.0,0.0,0.0), _bezierCurve->getPoint(_bezierCurve->getSelectedID()));
  drawSphere(vec3f(0.0,1.0,0.0), _bezierCurve->getPoint(_bezierCurve->getSelectedID()-1));
  }
  //				if(_bezierCurve->getSelectedID() != 0 && _bezierCurve->getSelectedID() != _bezierCurve->getNumPoints()-1)
  else
  {
  drawSphere(vec3f(1.0,0.0,0.0), _bezierCurve->getPoint(_bezierCurve->getSelectedID()));
  drawSphere(vec3f(0.0,1.0,0.0), _bezierCurve->getPoint(_bezierCurve->getSelectedID()-1));
  drawSphere(vec3f(0.0,1.0,0.0), _bezierCurve->getPoint(_bezierCurve->getSelectedID()+1));
  }
  }
  else
  {
  if(_bezierCurve->getSelectedID()%3 == 1)
  {
  drawSphere(vec3f(1.0,1.0,0.0), _bezierCurve->getPoint(_bezierCurve->getSelectedID()));
  }
  else
  {
  drawSphere(vec3f(1.0,1.0,0.0), _bezierCurve->getPoint(_bezierCurve->getSelectedID()));
  }

  }
  }
  }
  }

  void CubeFace(void)
  {
  glBegin (GL_QUADS);//Begin drawing state

  glNormal3f(  0.0f,  0.0f, -1.0f);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(	1.0f,  -1.0f, 0.0f);

  glNormal3f(  0.0f,  0.0f, -1.0f);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f( 1.0f, 1.0f, 0.0f);

  glNormal3f(  0.0f,  0.0f, -1.0f);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(  -1.0f, 1.0f, 0.0f);

  glNormal3f( 0.0f, 0.0f, -1.0f);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(	-1.0f, -1.0f, 0.0f);

  glEnd();//end drawing
  }

*/

/*
  void gSdlGl::visit(skybox* _box)
  {
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  //draw Skybox
  glPushMatrix();
  glTranslatef(_box->pos.i,_box->pos.j,_box->pos.k);
  glScalef(2,2,2);			//scale to prevent clipping
  glPushMatrix();
  glBindTexture(GL_TEXTURE_2D, skyFront);
  glTranslatef(0,0,-1);
  glRotatef(180,0,1,0);
  CubeFace(); //front
  glPopMatrix();

  glPushMatrix();
  glBindTexture(GL_TEXTURE_2D, skyBack);
  glTranslatef(0,0,1);
  CubeFace(); //back
  glPopMatrix();

  glPushMatrix();
  glBindTexture(GL_TEXTURE_2D, skyLeft);
  glTranslatef(-1,0,0);
  glRotatef(-90,0,1,0);
  CubeFace(); //left
  glPopMatrix();

  glPushMatrix();
  glBindTexture(GL_TEXTURE_2D, skyRight);
  glTranslatef(1,0,0);
  glRotatef(90,0,1,0);
  CubeFace(); //right
  glPopMatrix();

  glPushMatrix();
  glBindTexture(GL_TEXTURE_2D, skyTop);
  glTranslatef(0,1,0);
  glRotatef(180,0,1,0);
  glRotatef(-90,1,0,0);
  CubeFace(); //top
  glPopMatrix();

  glPushMatrix();
  glBindTexture(GL_TEXTURE_2D, skyBottom);
  glTranslatef(0,-1,0);
  glRotatef(180,0,1,0);
  glRotatef(90,1,0,0);
  CubeFace(); //bottom
  glPopMatrix();
  glPopMatrix();
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D);

  }*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
