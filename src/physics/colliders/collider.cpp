#include "collider.h"
#include "../collision.h"
#include "../rigid_body.h"

namespace physics
{
   namespace collider
   {
      std::vector<collider*> collider::m_colliders;
      collider::collider()
      {
	 collider::m_colliders.push_back(this);
      }
      void collider::depenetrate(vec3f newMTV)
      {
	 m_mtvs.push_back(newMTV);
	 m_mtv = newMTV;
      }
      const char* collider::get_debug_name()
      {
	 if (m_physics != nullptr)
	 {
	    return m_physics->get_debug_name();
	 }
	 return nullptr;
      }
      void collider::collide(const collider& /*collider*/)
      {
	 return;
      }
      void collider::add_collision(collision _collision)
      {
	 if(m_physics != nullptr)
	 {
	    m_physics->add_collision(_collision);
	 }
	 depenetrate(_collision.m_MTV);
      }
      void collider::recalculate(){return;}
      
      void collider::late_update(float /*_deltatime*/)
      {
	 m_mtv = vec3f();
	 for(auto tv : m_mtvs)
	 {
	    // if(Mathf.Abs(mtv.x) < Mathf.Abs(tv.x))
	    // 	mtv.x = tv.x;
	    // if(Mathf.Abs(mtv.y) < Mathf.Abs(tv.y))
	    // 	mtv.y = tv.y;
	    if(m_mtv.length_squared() < tv.length_squared())
	    {
	       m_mtv = tv;
	    }
	    // mtv += tv;
	 }
	 //Average if you're adding mtvs.
	 //if(mtvs.Count > 0) mtv /= mtvs.Count;
	 //todo: have a position that isn't just the rigid body position.
	 //      it's better.
	 if(m_physics != nullptr)
	 {
	    vec3f pos = m_physics->get_position();
	    m_physics->set_position(pos+m_mtv); //todo should subtract m_position here probably.
	 }
	 m_mtv = vec3f();
	 m_mtvs.clear();
      }
   }   
}
