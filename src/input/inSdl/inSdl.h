#ifndef INSDL_H
#define INSDL_H

#include "../input.h"
#include <SDL/SDL.h>

class inSdl : public input
{
 public:


  int init(void);
  bool isKeyPressed(keys _key);
  bool isMouseClicked(mouseButtons _button);
  bool update(void);
  void mouseDelta(float& _dx, float& _dy);
  bool setMousePosition(float _x, float _y);
  bool centerMousePosition();


 protected:
 private:
   };

#endif//INSDL_H
