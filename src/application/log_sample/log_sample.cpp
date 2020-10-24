#include "log_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../utils/string_helpers.h"
#include <string>
#include <cstring>

application* application::factory()
{
   return new log_sample();
}

void log_sample::run()
{
   fw::log::topics::add("log_sample");
   fw::log::topics::set_level("log_sample", fw::log::e_debug);
   auto topic = fw::log::scope("log_sample");
   auto total = fw::log::timer("total");
   {
      auto printfs = fw::log::timer("20000 printfs");
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
      auto logs = fw::log::timer("20000 logs");
      for(int i = 0; i < 20000; i++)
      {
   	 fw::log::info("asdf{}", i);
      }
   }
}
