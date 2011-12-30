#ifndef IRENDERVISITOR_H
#define IRENDERVISITOR_H

//#include "renderable/sprite/sprite.h"
//#include "renderable/sprite/animSprite/animSprite.h"

//Having these include here would lead to cirular inclusion.
//better to just have them included in the renderer.

class sprite;
class animSprite;

class iRenderVisitor
{
 public:

  virtual void visit(sprite* _sprite) = 0;
  virtual void visit(animSprite* _animSprite) = 0;

 protected:
 private:

};


#endif//IRENDERVISITOR_H
