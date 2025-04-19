#pragma once

class camera;

namespace physics
{
   namespace collider
   {
      class polygon;     
   }
}
//class Graph;
//class object3D;
//class bezierCurve;
//class coasterRider;
//class skybox;

namespace fw
{
   struct Mesh;
   struct Light;
}
struct ImDrawData;
class iRenderVisitor
{
 public:

  // virtual void visit(sprite* _sprite) = 0;
   virtual void visit(camera* _camera) = 0;
   virtual void visit(physics::collider::polygon* _poly) = 0;	 
   virtual void visit(fw::Mesh* _mesh) = 0;
   virtual void visit(fw::Light* _light) = 0;
   virtual void visit(ImDrawData* _imgui) = 0;
  //virtual void visit(Graph* _graph) = 0;
  //virtual void visit(object3D* _object3D) = 0;
  //virtual void visit(bezierCurve* _bezierCurve) = 0;
  //virtual void visit(coasterRider* _coasterRider) = 0;
  //virtual void visit(skybox* _skybox) = 0;

 protected:
 private:

};
