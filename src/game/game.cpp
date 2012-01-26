#include "game.h"
#include "../types/rect.h"
#include "../graphics/renderable/sprite/sprite.h"




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
  
  m_graphics->loadImage("hi.png");
  
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

  sprite mySprite;
  net::netEntity* me = new net::netEntity();

  /*if(!m_network->getType())
    {
      me = new net::netEntity;
      m_network->addEntity(me);
    }*/

  rect source;
  source.m_x = 0; source.m_y = 0;
  source.m_w = 16; source.m_h = 16;
  
  mySprite.m_imageCrop = source;
  mySprite.m_x = 25;
  mySprite.m_y = 25;
  mySprite.m_imageID = 0;

  while(m_looping)
    {
      if(m_input->update()) m_looping = false;

      mySprite.render(m_graphics->getRenderer());

    //  if(!m_network->getType())
	//{
    	  if(m_input->isKeyPressed(e_up))
	    {
	      me->setCommands(me->getCommands()|1);
	    }
	  else{me->setCommands(me->getCommands()&(255-1));}
    	  if(m_input->isKeyPressed(e_left))
	    {
	      me->setCommands(me->getCommands()|2);
	    }
	  else{me->setCommands(me->getCommands()&(255-2));}
    	  if(m_input->isKeyPressed(e_down))
	    {
	      me->setCommands(me->getCommands()|4);
	    }
	  else{me->setCommands(me->getCommands()&(255-4));}
    	  if(m_input->isKeyPressed(e_right))
	    {
	      me->setCommands(me->getCommands()|8);
	    }
	  else{me->setCommands(me->getCommands()&(255-8));}

    	  me->move();
	  mySprite.m_x = me->getXPos();
	  mySprite.m_y = me->getYPos();
	//}
      waitsecs(1.0f/60.0f);
      //SDL_Delay(30);
      //m_network->update(1.0f/60.0f);
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
