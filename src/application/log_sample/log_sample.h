#ifndef LOG_SAMPLE_H
#define LOG_SAMPLE_H
#include "../application.h"

class log_sample : public application
{
 public:
   void init(void) override;
   void run(void) override;
   void shutdown(void) override;
};
#endif//LOG_SAMPLE_H
