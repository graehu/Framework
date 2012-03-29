#include "game/game.h"
#include <windows.h>


int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR    lpCmdLine,
                     int       nCmdShow) 
{

  game fuzzyLogic;
  fuzzyLogic.run();

  return 0;
}
