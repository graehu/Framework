#include "game/game.h"
#include "application/application.h"
//#include <windows.h>




// int WINAPI WinMain(HINSTANCE hInstance,
//                      HINSTANCE hPrevInstance,
//                      LPSTR    lpCmdLine,
//                      int       nCmdShow)
int main()
{
  {
    application* l_app = application::mf_factory();
    l_app->mf_run();
  }

  return 0;
}
