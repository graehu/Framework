#ifndef GAME_H
#define GAME_H

#include <ctime>

#include "../application.h"

class net_physics_sample : public application
{
 public:
   void init(void) override;
   void run(void) override;
   void shutdown(void) override;
};

#endif//GAME_H
