#ifndef IRENDERVISITOR_H
#define IRENDERVISITOR_H

//Having these include here would lead to cirular inclusion.
//better to just have them included in the renderer.

class sprite;
class object3D;
class bezierCurve;

class iRenderVisitor
{
 public:

  virtual void visit(sprite* _sprite) = 0;
  virtual void visit(object3D* _object3D) = 0;
  virtual void visit(bezierCurve* _bezierCurve) = 0;

 protected:
 private:

};


#endif//IRENDERVISITOR_H
