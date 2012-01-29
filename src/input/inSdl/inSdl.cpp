#include "inSdl.h"


int inSdl::init()
{
  return 0;
}

bool inSdl::update(void)
{
  for(int i = 0; i < e_totalKeys; i++)
    m_keys[i] = false;

  char* internalKeys = (char*)SDL_GetKeyState(NULL);
    
  if (internalKeys['w'])
    m_keys[e_up] = true;
  if (internalKeys['a'])
    m_keys[e_left] = true;
  if (internalKeys['s'])
    m_keys[e_down] = true;
  if (internalKeys['d'])
    m_keys[e_right] = true;

  SDL_Event event;

  while (SDL_PollEvent(&event))
    {
      switch (event.type)
        {
	case SDL_QUIT:
	  {
	    return true;
	  }
        }
    }
  return false;
}

bool inSdl::isKeyPressed(gameKeys _key)
{
  return m_keys[_key];
}


void inSdl::mouseDelta(float& _x, float& _y)
{
	int x = 0;
	int y = 0;

	SDL_GetRelativeMouseState(&x, &y);
	_x = x;
	_y = y;
}



input* input::inputFactory()
{
  return (input*)new inSdl; //returns an input class of sdl type
}

