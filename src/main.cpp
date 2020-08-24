#include "application/application.h"
#include "utils/params.h"

//#include <windows.h>




// int WINAPI WinMain(HINSTANCE hInstance,
//                      HINSTANCE hPrevInstance,
//                      LPSTR    lpCmdLine,
//                      int       nCmdShow)
int main(int argc, char *argv[])
{
   commandline::arg_count = argc;
   commandline::arg_variables = argv;
   application* l_app = application::mf_factory();
   l_app->mf_run();
   return 0; 
}
