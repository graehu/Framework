#include "collision_manager.h"
#include "colliders/collider.h"
#include "rigid_body.h"
#include "../utils/log/log.h"

namespace physics
{
   void collision_manager::update()
   {
      fw::log::scope collision_manager("collision_manager", true);
      fw::log::timer col_update("col_update");
      {
	 fw::log::timer col_cols("col_cols");
	 for(unsigned int i = 0; i < collider::collider::m_colliders.size(); i++)
	 {
	    collider::collider* current = collider::collider::m_colliders[i];
	    if(i == 0)
	    {
	       current->recalculate();
	    }
	    if(current->m_physics == nullptr) continue;
	    for (unsigned int ii = 0; ii < collider::collider::m_colliders.size(); ii++)
	    {
	       collider::collider* other = collider::collider::m_colliders[ii];
	       if(i == 0)
	       {
		  other->recalculate();
	       }
	       // this was wrong, and expensive to calculate.
	       // if(other->m_collided.find(current) != other->m_collided.end()) continue;
	       // other->m_collided.insert(current);
	       // current->m_collided.insert(other);
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
	 for(unsigned int i = 0; i < collider::collider::m_colliders.size(); i++)
	 {
	    collider::collider::m_colliders[i]->late_update(0);
	 }
      }
      {
	 fw::log::timer col_resolve("col_resolve");
	 for(unsigned int i = 0; i < collider::collider::m_colliders.size(); i++)
	 {
	    collider::collider* current = collider::collider::m_colliders[i];
	    // current->m_collided.clear();
	    if(current->m_physics != nullptr)
	    {
	       current->m_physics->resolve_collisions();
	    }
	 }
      }
   }
}
