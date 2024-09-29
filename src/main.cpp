#include "application/application.h"
#include "utils/params.h"

//#include <windows.h>




// int WINAPI WinMain(HINSTANCE hInstance,
//                      HINSTANCE hPrevInstance,
//                      LPSTR    lpCmdLine,
//                      int       nCmdShow)
int main(int argc, char *argv[])
{
   fw::commandline::arg_count = argc;
   fw::commandline::arg_variables = argv;
   application* app = application::factory();
   app->init();
   app->run();
   app->shutdown();
   return 0; 
}
