#include "game.h"
#include "../graphics/renderable/sprite/sprite.h"
#include "../physics/rigidBody.h"
#include "../physics/polygon.h"
#include <windows.h>
#include <cassert>

game::game()
{
  m_looping = true;
  m_name = "physics";
}

void game::init(void)
{
  m_window = window::windowFactory();
  m_window->init(512, 512, m_name);
  m_graphics = graphics::graphicsFactory();
  m_graphics->init();
  m_input = input::inputFactory();
  m_input->init();
}

void game::run(void)
{
  init();
  sprite mahSprite, tester, center;
  center.m_position = vec3f(2,2);
  center.m_fileName = tester.m_fileName = mahSprite.m_fileName = "assets/car.bmp";
  mahSprite.m_position = vec3f(0.2f, 0.2f);

  polygon polyOne;
  polygon polyTwo;

  polyOne.m_vertices.push_back(vec3f(-1.0f,-1.0f));
  polyOne.m_vertices.push_back(vec3f(-1.0f,1.0f));
  polyOne.m_vertices.push_back(vec3f(1.0f,1.0f));
  polyOne.m_vertices.push_back(vec3f(1.0f,-1.0f));

  polyTwo.m_vertices = polyOne.m_vertices;

  rigidBody test[10];
  for(unsigned int i = 0; i < 10; i++)
	test[i].m_vertices = polyOne.m_vertices;

  float time = 0;
  float dt = 1.0f/30.0f;
  

  while(m_looping)
	{
	  if(m_input->update()) m_looping = false;

	  for(unsigned int i = 0; i < 10; i++)
	  {
		  test[i].render(m_graphics->getRenderer());
  		  test[i].update(time+i, dt);
	  }
	  time += dt;

	  polyTwo.render(m_graphics->getRenderer());
	  polyOne.render(m_graphics->getRenderer());


	  if(m_input->isKeyPressed(input::e_left))
		  for(unsigned int i = 0; i < polyTwo.m_vertices.size(); i++)
			  polyTwo.m_vertices[i].i -= 0.1f;

	  if(m_input->isKeyPressed(input::e_right))
		  for(unsigned int i = 0; i < polyTwo.m_vertices.size(); i++)
			  polyTwo.m_vertices[i].i += 0.1f;

	  if(m_input->isKeyPressed(input::e_up))
		  for(unsigned int i = 0; i < polyTwo.m_vertices.size(); i++)
			  polyTwo.m_vertices[i].j += 0.1f;

	  if(m_input->isKeyPressed(input::e_down))
		  for(unsigned int i = 0; i < polyTwo.m_vertices.size(); i++)
			  polyTwo.m_vertices[i].j -= 0.1f;



	  polyOne.collideSAT(&polyTwo);

	  m_graphics->render();
	  
	  Sleep(30);
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