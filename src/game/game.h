#ifndef GAME_H
#define GAME_H

#include <ctime>

#include "../window/window.h"
#include "../graphics/graphics.h"
#include "../input/input.h"


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

 private:

  void init(void);

};

#endif//GAME_H
