#ifndef COLLISION_MANAGER_H
#define COLLISION_MANAGER_H

#include "colliders/collider.h"
#include "rigid_body.h"
#include "../utils/log/log.h"

namespace physics
{
   class collision_manager
   {
     public:
      //Some sort of space partitioning.
      //all of the bodies.
      //on conllsion add collision data to bodies.
      //bodies move themselves.
      //work out next frames movement for bodies and do collisions based on that?
      //multi sampled collisions
      static void update()
      {
	 fw::log::timer col_update("col_update");
	 {
	    fw::log::timer col_cols("col_cols");
	    for(int i = 0; i < collider::collider::m_colliders.size(); i++)
	    {
	       collider::collider* current = collider::collider::m_colliders[i];
	       if(i == 0)
	       {
		  current->recalculate();
	       }
	       if(current->m_physics == nullptr) continue;
	       for (int ii = 0; ii < collider::collider::m_colliders.size(); ii++)
	       {
		  collider::collider* other = collider::collider::m_colliders[ii];
		  if(i == 0)
		  {
		     other->recalculate();
		  }
		  if(other->m_collided.find(current) != other->m_collided.end()) continue;
		  other->m_collided.insert(current);
		  current->m_collided.insert(other);
		  if(other != current)
		  {
		     if(current->m_bounds.touching(other->m_bounds))
		     {
			current->collide(*other);
		     }
		  }
	       }
	    }
	 }
	 {
	    fw::log::timer col_late("col_late_update");
	    for(int i = 0; i < collider::collider::m_colliders.size(); i++)
	    {
	       collider::collider::m_colliders[i]->late_update(0);
	    }
	 }
	 {
	    fw::log::timer col_resolve("col_resolve");
	    for(int i = 0; i < collider::collider::m_colliders.size(); i++)
	    {
	       collider::collider* current = collider::collider::m_colliders[i];
	       current->m_collided.clear();
	       if(current->m_physics != nullptr)
	       {
		  current->m_physics->resolve_collisions();
	       }
	    }
	 }
      }
   };
}
#endif//COLLISION_MANAGER_H
