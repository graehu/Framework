#ifndef INSDL_H
#define INSDL_H

#include "../input.h"
#include <SDL/SDL.h>

class inSdl : public input
{
 public:


  int init(void);
  bool isKeyPressed(gameKeys _key);
  bool update(void);
  int mouse(void); //hmmmmmmmmmmmmmmm

 protected:

 private:
   };

#endif//INSDL_H
