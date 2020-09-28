#include "polygon.h"
#include "../collision.h"
#include "../rigid_body.h"


namespace physics
{
   namespace collider
   {
      void polygon::collide(const collider& _collider)
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
	 catch(std::bad_cast _exception)
	 {
	 }
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
	 catch(std::bad_cast _exception)
	 {
	 }
      }
      
      void polygon::recalculate()
      {
	 m_bounds.extents = vec3f();
	 if(m_physics != nullptr)
	 {
	    // printf("re cal p %p\n", this);	 
	    m_bounds.center = m_position+m_physics->get_position();
	 }
	 else
	 {
	    // printf("re cal n %p\n", this);	
	    m_bounds.center = m_position;
	 }
	 if (m_transformed_vertices.size() != m_vertices.size())
	 {
	    m_transformed_normals.resize(m_vertices.size());
	    m_transformed_vertices.resize(m_vertices.size());  
	 }
	 //todo: add a transform to this physics stuff?
	 for(int i = 0; i < m_vertices.size(); i++)
	 {
	    if(m_physics != nullptr)
	    {
	       m_transformed_vertices[i] = m_physics->get_transform().to_local(m_vertices[i])+m_position;
	    }
	    else
	    {
	       m_transformed_vertices[i] = m_vertices[i]+m_position;
	    }
	    vec3f pointA = m_transformed_vertices[i];
	    vec3f pointB = m_transformed_vertices[(i+1)%m_vertices.size()];
	    vec3f edgeDir = pointB-pointA;
	    edgeDir.normalise_self();
	    // if((transform.localScale.x < 0f && transform.localScale.y > 0f) ||
	    // 	  (transform.localScale.y < 0f && transform.localScale.x > 0f))
	    // { calcNorms[i] = -(new Vector2(-edgeDir.y, edgeDir.x)); }
	    // else {
	    m_transformed_normals[i] = vec3f(-edgeDir.i, edgeDir.j);
	    // }
	    m_bounds.encapsulate(m_transformed_vertices[i]);
	 }
      }  
   }
}
