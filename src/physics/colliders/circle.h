#ifndef CIRCLE_H
#define CIRCLE_H

#include "../../types/vec2f.h"
#include "collider.h"

namespace physics
{
   namespace collider
   {
      class circle : public collider
      {
	public:
	 
	 vec2f m_center;
	 float m_radius;
	 void collide(const collider& _collidable) final;
	 void add_collision(collision _collision) final;
      };
   }
}
#endif//CIRCLE_H
