#ifndef IRENDERABLE_H
#define IRENDERABLE_H

#include "../iRenderVisitor.h"

class iRenderable
{
 public:

  virtual void render(iRenderVisitor* _renderer) = 0;

 protected:
 private:

};

#endif//IRENDERABLE_H
