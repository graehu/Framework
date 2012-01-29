#include "game.h"
#include "../types/rect.h"
#include "../graphics/renderable/sprite/sprite.h"
#include <GL/glu.h>

#include "../graphics/renderable/3DObject/object3D.h"

game::game()
{
  m_looping = true;
  m_name = "Gradius";
}

void game::init(void)
{
  m_window = window::windowFactory();
  m_window->init(500, 500, m_name);

  m_graphics = graphics::graphicsFactory();
  m_graphics->init();
  
  m_input = input::inputFactory();
  m_input->init();
  

  
  //m_network = new net::network(0xF00D, 100.0f);
  /*
  if(m_network->init(true, 8000) == 1)
    {
      bool unbound = true;
      int i=1;
      while(unbound)
	{
	  if(m_network->init(false, 8000+i) == 0)
	    {
	      unbound = false;
	    }
	  i++;
	}
    }
    */


}

void game::run(void)
{
  init();

  object3D* mahObject = new object3D("data/poolTable.ms3d");
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glLightModelf(GL_LIGHT_MODEL_AMBIENT, (1.0, 1.0, 1.0, 1.0));

  while(m_looping)
    {
      if(m_input->update()) m_looping = false;
	  

       	  if(m_input->isKeyPressed(e_up))
			m_camera.changeForwardVelocity(0.001f);
    	  if(m_input->isKeyPressed(e_down))
			m_camera.changeForwardVelocity(-0.001f);
		  if(m_input->isKeyPressed(e_right))
			m_camera.changeStrafeVelocity(0.001f);
    	  if(m_input->isKeyPressed(e_left))
			m_camera.changeStrafeVelocity(-0.001f);
			 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear The Screen And The Depth Buffer
		  mahObject->render(m_graphics->getRenderer());


		  float x, y;
		  m_camera.update();
		  m_input->mouseDelta(x,y);
		  m_camera.changeHeading(x);
		  m_camera.changePitch(y);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixf((GLfloat*)m_camera.getView().elem);

      waitsecs(1.0f/60.0f);
      m_graphics->render();
    }
}

void game::notify(net::events _event)
{
  switch(_event)
    {
    case net::e_newEntityEvent:
      //push back new entity
      break;
    }
}




game::~game()
{

}

















///find a better place to do hit detection you foo'

/*
  bool game::hitTest(square sOne, square sTwo)
  {
  int x1 = *(sOne.getPosX());
  int y1 = *(sOne.getPosY());
  int height1 = *(sOne.getHeight());
  int width1 = *(sOne.getWidth());

  int x2 = *(sTwo.getPosX());
  int y2 = *(sTwo.getPosY());
  int height2 = *(sTwo.getHeight());
  int width2 = *(sTwo.getWidth());

  if (((x1 + width1 > x2) && (x1 < (x2 + width2))) && ((y1 + height1 > y2) && (y1 < (y2 + height2))))
  {
  return true;
  }
  else
  {
  return false;
  }
  }


*/