#ifndef MPEG_SAMPLE_H
#define MPEG_SAMPLE_H
#include "../application.h"

class mpeg_sample : public application
{
 public:
   void init(void) override;
  void run(void) override;
   void shutdown(void) override;
};

#endif//MPEG_SAMPLE_H
