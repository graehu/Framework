#ifndef RCSAMPLE_H
#define RCSAMPLE_H
#include "../application.h"

class rc_sample : public application
{
 public:
  void run(void) override;
   void init(void) override;
   void shutdown(void) override;
};
#endif//RCSAMPLE_H
