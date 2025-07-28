#ifndef MODULE_SAMPLE_H
#define MODULE_SAMPLE_H
#include "../application.h"

class module_sample : public application
{
 public:
  void run(void) override;
   void init(void) override;
   void shutdown(void) override;
};
#endif//MODULE_SAMPLE_H
