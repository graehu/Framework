#ifndef BITMAP_SAMPLE_H
#define BITMAP_SAMPLE_H
#include "../application.h"

class bitmap_sample : public application
{
 public:
   void init(void) override;
   void run(void) override;
   void shutdown(void) override;
   
};
#endif//BITMAP_SAMPLE_H
