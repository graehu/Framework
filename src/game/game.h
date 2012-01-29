#ifndef GAME_H
#define GAME_H

#include <ctime>

#include "../window/window.h"
#include "../graphics/graphics.h"
#include "../input/input.h"
#include "../graphics/camera/camera.h"
#include "../networking/net.h"
#include "../networking/utils/subscriber.h"




class game : public net::subscriber
{
 public:

  game();
  ~game();

  void run(void);
  void notify(net::events _event);

 protected:

  bool m_looping;
  char* m_name;
  graphics* m_graphics;
  window* m_window;
  input* m_input;
  camera m_camera;
  //net::network* m_network;

 private:

  void init(void);

};

#endif//GAME_H
