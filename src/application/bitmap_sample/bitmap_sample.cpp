#include "bitmap_sample.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include "../../graphics/resources/bitmap.h"
//STD includes
using namespace fw;
application* application::factory()
{
   return new bitmap_sample();
}

void bitmap_sample::init()
{
   commandline::parse();
   log::topics::add("bitmap_sample");
   params::add("bitmap", {"test.bmp"});
}

void bitmap_sample::run(void)
{
   log::scope topic("bitmap_sample");
   log::info("----------------");
   bitmap local(params::get_value("bitmap", 0));
   if(local.save("out.bmp"))
   {
      log::info("bitmap successfully copied");
   }
   else
   {
      log::error("bitmap copy failed");
   }
}
void bitmap_sample::shutdown(void) {}
