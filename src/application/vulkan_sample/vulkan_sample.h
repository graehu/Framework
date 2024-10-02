#pragma once
#include "../application.h"

class vulkan_sample : public application
{
 public:
   void init(void) override;
   void run(void) override;
   void shutdown(void) override;
};
