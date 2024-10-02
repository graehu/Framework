#ifndef NEW_SAMPLE_H
#define NEW_SAMPLE_H
#include "../application.h"

class new_sample : public application
{
 public:
  void run(void) override;
   void init(void) override;
   void shutdown(void) override;
};
#endif//NEW_SAMPLE_H
