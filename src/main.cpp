#include "application/application.h"
#include "utils/params.h"



int main(int argc, char *argv[])
{
   fw::commandline::arg_count = argc;
   fw::commandline::arg_variables = argv;
   application* app = application::factory();
   if(app != nullptr)
   {
      app->init();
      app->run();
      app->shutdown();
   }
   return 0; 
}


#ifdef _WIN32
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR    lpCmdLine,
                     int       nCmdShow)
{
   LPWSTR *argv = nullptr;
   int argc = 0;

   argv = CommandLineToArgvW(GetCommandLineW(), &argc);
   int ret = main(argc, argv);

   LocalFree(szArglist);

   return ret;
}

#endif
