#include "game.h"
#include <SDL/SDL.h>
#include "../types/rect.h"
#include "../graphics/renderable/sprite/sprite.h"


game::game()
{
  m_looping = true;
}

void game::init(void)
{
  m_window = window::windowFactory();
  m_window->init(200, 200, "Gradius");

  m_graphics = graphics::graphicsFactory();
  m_graphics->init();
  
  m_input = input::inputFactory();
  m_input->init();
  
  m_graphics->loadImage("image.bmp");
  
}

void game::run(void)
{
  init();

  sprite mySprite;

  rect source;
  source.m_x = 0; source.m_y = 0;
  source.m_w = 128; source.m_h = 128;
  
  mySprite.m_imageCrop = source;
  mySprite.m_x = 25;
  mySprite.m_y = 25;
  mySprite.m_imageID = 0;

  while(m_looping)
    {
      if(m_input->update()) m_looping = false;
      //m_graphics->blitImage(0, source, destination);
      //mySprite.render((iRenderVisitor*)m_graphics);
      mySprite.render(m_graphics->getRenderer());
      SDL_Delay(30);
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
