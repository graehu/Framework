#ifndef INSIMPLE_H
#define INSIMPLE_H
#include "../input.h"

namespace std
{
  class thread;
}
class inSimple : public input
{
 public:
  inSimple();
  ~inSimple();
  int init(void); 
  bool update(void);
  bool isKeyPressed(keys _key);
  bool isMouseClicked(mouseButtons _button);
  void mouseDelta(float& _dx, float& _dy);

  // static input* inputFactory(void);
protected:
  void pollInput();
  bool m_dying;
  std::thread* m_thread;

};

#endif/*INSIMPLE_H*/
