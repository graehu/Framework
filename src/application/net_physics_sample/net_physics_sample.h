#ifndef GAME_H
#define GAME_H

#include <ctime>

#include "../../window/window.h"
#include "../../graphics/graphics.h"
#include "../../input/input.h"
#include "../../networking/net.h"
#include "../application.h"

class net_physics_sample : public application
{
 public:

  net_physics_sample();
  void run(void) override;

 protected:

  bool m_looping;
  const char* m_name;

  graphics* m_graphics;
  window* m_window;
  input* m_input;
  net::network* m_network;

 private:

  void init(void);


};

#endif//GAME_H
