#ifndef SPRITE_H
#define SPRITE_H

#include "../../../types/rect.h"
#include "../iRenderable.h"

class sprite : public iRenderable
{
 public:
  sprite(char* _filename = "null"){m_x = m_y = m_z = 0; m_fileName = _filename;}
  ~sprite(){}

  void render(iRenderVisitor* _renderer)
  {
    _renderer->visit(this);
  }
  float m_x, m_y, m_z;
  char* m_fileName; // This is the file it's in.
  rect m_imageCrop; // This is how much of the image we want.
 protected:

 private:
};

#endif//SPRITE_H
