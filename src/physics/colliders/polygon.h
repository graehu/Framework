#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include "collider.h"

namespace physics
{
   namespace collider
   {
      ///A Generic polygon shape class, used for colidable areas.
      class polygon : public collider
      {
      public:
	 float length() const { return m_vertices.size(); }
	 ///Renders this polygon, wireframe
	 virtual void collide(const collider& collider);
	 void recalculate() final;
	 void render(iRenderVisitor* _renderer) { _renderer->visit(this); }
	 ///Stores the vertices that make up the body.
	 std::vector<vec3f> m_vertices;
	 std::vector<vec3f> m_normals;
      protected:

      private:
      };
   }
}
#endif//POLYGON_H
