#include "scratch_sample.h"
#include "../../utils/log/log.h"

application* application::factory()
{
   return new scratch_sample();
}

void scratch_sample::run(void)
{
   
}
