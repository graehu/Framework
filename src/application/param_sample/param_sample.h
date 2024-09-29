#ifndef PARAM_SAMPLE_H
#define PARAM_SAMPLE_H
#include "../application.h"

class param_sample : public application
{
 public:
   void init(void) override;
   void run(void) override;
   void shutdown(void) override;
};
#endif//PARAM_SAMPLE_H
