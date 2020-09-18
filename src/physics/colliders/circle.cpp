#include "circle.h"
#include "../collision.h"
#include <typeinfo>


namespace physics
{
   namespace collider
   {
      void circle::collide(const collider& _collider)
      {
	 //todo: this is a stopgap implementation, avoid cast/throw.
	 try
	 {
	    const polygon& poly = dynamic_cast<const polygon&>(_collider);
	    collision col = collision::sat(*this, poly);
	    if(col.m_hit)
	    {
	       add_collision(col);  
	    }
	    return;
	 }
	 catch(std::bad_cast _exception) { }
	 try
	 {
	    const circle& other_circle = dynamic_cast<const circle&>(_collider);
	    collision col = collision::sat(*this, other_circle);
	    if(col.m_hit)
	    {
	       add_collision(col);  
	    }
	    return;
	 }
	 catch(std::bad_cast _exception) { }
      }
      void circle::add_collision(collision _collision)
      {
	 //TODO: Something like this needs to go back in.
	 // 	collision.partner = poly;
	 // collision.MTV *= -1f;
	 // collision.normal *= -1f;
	 collider::add_collision(_collision);
      }
   }
}

