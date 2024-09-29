#include "log_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../utils/string_helpers.h"
#include <string>
#include <cstring>

using namespace fw;

application* application::factory()
{
   return new log_sample();
}
void log_sample::init(){}
void log_sample::run()
{
   commandline::parse();
   log::scope log_sample("log_sample", true);
   log::timer total("total");
   {
      log::timer printfs("20000 printfs");
      for(int i = 0; i < 20000; i++)
      {
	 #ifndef NO_LOG_LABELS
   	 printf("%sasdf%d\n", "[log_sample] ", i);
	 #else
	 printf("asdf%d\n", i);
	 #endif
      }
   }
   {
      log::timer logs("20000 logs");
      for(int i = 0; i < 20000; i++)
      {
   	 log::info("asdf{}", i);
      }
   }
}
void log_sample::shutdown(){}
