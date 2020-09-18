#include "polygon.h"
#include "../collision.h"

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
      
      void polygon::recalculate()
      {
	 // lastTransform = new StoreTransform(transform);
	 m_bounds.extents = vec3f();
	 m_bounds.center = m_position;
	 //TODO: Optimise this, the list doesn't need to be itterated twice.
	 //todo: add a transform to this physics stuff?
	 // for(int i = 0; i < m_vertices.Length; i++)
	 // {
	 // calcVerts[i].x = vertices[i].x;
	 // calcVerts[i].y = vertices[i].y;
	 // calcVerts[i].x = (calcVerts[i].x*lastTransform.localScale.x);
	 // calcVerts[i].y = (calcVerts[i].y*lastTransform.localScale.y);
	 // calcVerts[i] = lastTransform.rotation*calcVerts[i];
	 // calcVerts[i] += (Vector2)lastTransform.position;
	 // bounds.Encapsulate(calcVerts[i]);
	 // }
	 //Normals
	 for(int i = 0; i < m_vertices.size(); i++)
	 {
	    vec3f pointA = m_vertices[i];
	    vec3f pointB = m_vertices[(i+1)%m_vertices.size()];
	    vec3f edgeDir = pointB-pointA;
	    edgeDir.normalise_self();
	    // if((transform.localScale.x < 0f && transform.localScale.y > 0f) ||
	    // 	  (transform.localScale.y < 0f && transform.localScale.x > 0f))
	    // { calcNorms[i] = -(new Vector2(-edgeDir.y, edgeDir.x)); }
	    // else {
	    m_normals[i] = vec3f(-edgeDir.i, edgeDir.j);
	    // }
	    m_bounds.encapsulate(pointA);
	 }
      }  
   }
}
