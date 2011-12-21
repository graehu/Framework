#ifndef IRENDERVISITOR_H
#define IRENDERVISITOR_H

#include "renderable/sprite/sprite.h"
#include "renderable/sprite/animSprite/animSprite.h"
//class iRenderVisitor;
class iRenderVisitor
{
 public:

  //  iRenderVisitor(){};
  //  ~iRenderVisitor(){};

  virtual void visit(sprite* _sprite) = 0;
  virtual void visit(animSprite* _animSprite) = 0;

 protected:
 private:

};


#endif//IRENDERVISITOR_H
