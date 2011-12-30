#ifndef ANIMSPRITE_H
#define ANIMSPRITE_H

#include "../sprite.h"

class animSprite : public sprite
{
 public:
  animSprite();
  ~animSprite();

  //animation controls go here.
  void render(iRenderVisitor* _renderer){_renderer->visit(this);}

 protected:

 private:
};


#endif//ANIMSPRITE_H
