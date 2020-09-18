#ifndef COLLIDER_H
#define COLLIDER_H
#include "../../types/bounds.h"
#include "../../types/vec3f.h"
#include "../../application/update.h"
#include "../../graphics/renderable/iRenderable.h"


#include <vector>
#include <set>

namespace physics
{
   class collision;
   class rigid_body;
   namespace collider
   {
      class collider : public iRenderable
      {
      public:
	 collider();
	 void depenetrate(vec3f newMTV);
	 virtual void add_collision(collision collision);
	 virtual void collide(const collider& collider);
	 virtual void recalculate();
	 void late_update(float _deltatime);
	 virtual void render(iRenderVisitor* _renderer) {}
	 //types
	 bool m_debug = false;
	 bool m_kinematic = false;
	 rigid_body* m_physics = nullptr;
	 vec3f m_position;
	 static std::vector<collider*> m_colliders;
	 std::set<collider*> m_collided;
	 bounds m_bounds;
	 vec3f m_mtv;
	 std::vector<vec3f> m_mtvs;
      };   
   }
}
#endif//COLLIDER_H
