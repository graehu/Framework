#include "game.h"
#include "../graphics/renderable/sprite/sprite.h"
#include "../physics/rigidBody.h"
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
  sprite mahSprite;
  mahSprite.m_fileName = "assets/car.bmp";
  mahSprite.m_x = 12;
  mahSprite.m_y = 12;
  rigidBody mahBody;

  while(m_looping)
    {
      if(m_input->update()) m_looping = false;
	  mahSprite.render(m_graphics->getRenderer());
	  Sleep(30);
      m_graphics->render();
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