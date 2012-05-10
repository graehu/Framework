#ifndef GAME_H
#define GAME_H

#include <ctime>

#include "../window/window.h"
#include "../graphics/graphics.h"
#include "../input/input.h"
#include "../networking/net.h"

class game
{
 public:

  game();
  ~game();

  void run(void);

 protected:

  bool m_looping;
  char* m_name;

  graphics* m_graphics;
  window* m_window;
  input* m_input;
  net::network* m_network;

 private:

  void init(void);


};

#endif//GAME_H
