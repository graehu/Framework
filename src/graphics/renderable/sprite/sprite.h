#ifndef SPRITE_H
#define SPRITE_H

#include "../../../types/rect.h"
//#include "../../iRenderVisitor.h" // i shouldn't need this...
#include "../iRenderable.h"

class sprite : public iRenderable
{
 public:

  sprite(){m_imageID = 0; m_x = m_y = 0;}
  ~sprite(){};

  void render(iRenderVisitor* _renderer)
  {
    sprite* odd = this;
    _renderer->visit(odd);
  }

  float m_x, m_y;
  int m_imageID; // This is the image we want.
  rect m_imageCrop; // This is how much of the image we want.

 protected:

 private:
};

#endif//SPRITE_H
