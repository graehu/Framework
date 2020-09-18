#ifndef COLLISION_H
#define COLLISION_H

#include "colliders/polygon.h"
#include "colliders/circle.h"
#include <vector>
#include <algorithm>

template <typename T> int sign(T val)
{
    return (T(0) < val) - (val < T(0));
}

namespace physics
{
   class collision
   {
   public:
      bool m_hit = false;
      vec3f m_normal;//surface normal
      vec3f m_point;//point of collision
      vec3f m_MTV; //minimum translation vector
      static collision sat(const collider::circle& circle1, const collider::circle& circle2)
      {
	 collision collision;
	 float summed_radius = circle1.m_radius+circle2.m_radius;
	 float distance_squared = (circle1.m_center.i-circle2.m_center.i)*(circle1.m_center.i-circle2.m_center.i);
	 distance_squared += (circle1.m_center.j-circle2.m_center.j)*(circle1.m_center.j-circle2.m_center.j);
	 if(distance_squared < summed_radius*summed_radius)
	 {
	    float difference = summed_radius-sqrt(distance_squared);
	    collision.m_MTV = vec3f((circle2.m_center.i - circle1.m_center.i)*difference, (circle2.m_center.j - circle1.m_center.j)*difference);
	    collision.m_MTV = -collision.m_MTV;
	    collision.m_normal = collision.m_MTV.normalise();
	    collision.m_hit = true;
	 }
	 return collision;
      }
      static collision sat(const collider::polygon& poly1, const collider::polygon& poly2)//, bool debug = false)
      {
	 collision collision;
	 collider::polygon polys[] = {poly1, poly2};
	 auto num_polys = sizeof(polys)/sizeof(collider::polygon);
	 float  minOverlap = -99999.0f;
	 std::vector<vec3f> tested;
	 for(int i = 0; i < num_polys; i++)
	 {
	    for(int ii = 0; ii < polys[i].m_vertices.size(); ii++)
	    {
	       vec3f normal = polys[i].m_normals[ii];
	       if(std::find(tested.begin(), tested.end(), normal) != tested.end())
	       {
		  continue;
	       }
	       tested.push_back(normal);
	       //Find the projected shape's ranges on the normal.
	       vec3f range1 = project_points(normal, polys[i].m_vertices);// , debug);
	       vec3f range2 = project_points(normal, polys[(i+1)%2].m_vertices);
	       if(i == 0 && !collide_ranges(range1, range2, -normal, collision, minOverlap)) return {};
	       else if(!collide_ranges(range1, range2, normal, collision, minOverlap)) return {};
	    }
	 }
	 collision.m_hit = true;
	 return collision;
      }
      //SAT + Voronoi regions
      static collision sat(const collider::circle& _circle, const collider::polygon& poly)// , bool debug = false)
      {
	 collision col = sat2(poly, _circle);// , debug);
	 if(col.m_hit)
	 {
	    return col;
	 }
	 return {};
      }
      static collision sat(const collider::polygon& poly, const collider::circle& circle)//, bool debug = false)
      {
	 collision col = sat2(poly, circle);//, debug);
	 if(col.m_hit)
	 {
	    col.m_MTV *= -1.0f;
	    col.m_normal *= -1.0f;
	    return col;
	 }
	 return {};
      }
   protected:
      static collision sat2(const collider::polygon& _poly, const collider::circle& circle)//, bool debug = false)
      {
	 collision collision;
	 float  minOverlap = -99999.0f;
	 std::vector<vec3f> tested;
	 vec3f testVert;
	 for(int i = 0; i < _poly.m_vertices.size(); i++)
	 {
	    vec3f normal = _poly.m_normals[i];
	    if(std::find(tested.begin(), tested.end(), normal) != tested.end())
	    {
	       continue;
	    }
	    tested.push_back(normal);
	    // Debug.Log("i = " + i + " i-1 = " + ((i+poly.Length-1)%poly.Length));
	    //todo: find a better way construct vec3 from vec2 while mainitaining vec2 to vec3?
	    vec3f prevNorm = _poly.m_normals[(i+_poly.m_vertices.size()-1)%_poly.m_vertices.size()];
	    float side1 = collision.find_point_side(circle.m_center, vec3f(_poly.m_vertices[i].i, _poly.m_vertices[i].j), prevNorm);
	    float side2 = collision.find_point_side(circle.m_center, vec3f(_poly.m_vertices[i].i, _poly.m_vertices[i].j), normal);
	    if(side1 == -1 && side2 == 1)
	    {
	       testVert = _poly.m_vertices[i];
	       // if(debug)
	       // {
	       // 	  Debug.DrawLine(testVert, testVert+prevNorm, Color.cyan);
	       // 	  Debug.DrawLine(testVert, testVert+normal, Color.cyan);
	       // }
	    }
	    //Find the projected shape's ranges on the normal.
	    vec3f range1 = project_points(normal, _poly.m_vertices);
	    vec3f range2 = project_circle(normal, circle.m_center, circle.m_radius);

	    if(!collide_ranges(range1, range2, normal, collision, minOverlap)) return {};
	 }
	 //Test Voronoi region
	 if(testVert != vec3f())
	 {
	    vec3f normal = testVert-circle.m_center;
	    normal.normalise_self();
	    //Find the projected shape's ranges on the normal.
	    vec3f range1 = project_points(normal, _poly.m_vertices);
	    vec3f range2 = project_circle(normal, circle.m_center, circle.m_radius);

	    if(!collide_ranges(range1, range2, normal, collision, minOverlap)) return {};
	 }
	 return collision;
      }
      //If there's no MTV return false
      //Otherwise set the MTV if it's the minimum overlap
   private:
      static bool collide_ranges(vec3f range1, vec3f range2, vec3f normal, collision& collision, float& minOverlap)
      {
	 //NOTE: Overlaps are both negative if overlapping
	 float overlap1 = range1.i - range2.j; //overlap 1
	 float overlap2 = range2.i - range1.j; //overlap 2

	 if(overlap1 > 0.0f || overlap2 > 0.0f) { return false; }
	 else if(overlap1 > minOverlap || overlap2 > minOverlap)
	 {
	    if(overlap1 > overlap2)
	    {
	       minOverlap = overlap1;
	       collision.m_MTV = -normal*-minOverlap;
	       collision.m_normal = -normal;
	    }
	    else
	    {
	       minOverlap = overlap2;
	       collision.m_MTV = normal*-minOverlap;
	       collision.m_normal = normal;
	    }
	 }
	 return true;
      }
      static vec3f project_points(const vec3f& projection, const std::vector<vec3f>& points, bool debug = false)
      {
	 float diff = points[0].dot_product2(projection);
	 vec3f range = vec3f(diff, diff);
	 for(int i = 0; i < points.size(); i++)
	 {
	    diff = points[i].dot_product2(projection);
	    if (diff < range.i) range.i = diff;
	    else if(diff > range.j) range.j = diff;
	 }
	 // if(debug)
	 // {
	 //    Debug.DrawLine(projection*range.x, projection*range.y, new Color (1f, 0.92f, 0.016f, 0.5f));  
	 // }
	 return range;
      }
      static vec3f project_circle(vec3f projection, vec3f center, float radius)// , bool debug = false)
      {
	 float projCenter = center.dot_product2(projection);
	 vec3f range(-radius+projCenter, radius+projCenter);
	 // if(debug)
	 // {
	 //    Debug.DrawLine(projection*range.x, projection*range.y, new Color (1f, 0.92f, 0.016f, 0.5f));  
	 // }
	 return range;
      }
      static float find_point_side(vec3f point, vec3f origin, vec3f direction)
      {
	 vec3f dir = origin+direction;
	 return sign( (dir.i-origin.i)*(point.j-origin.j) - (dir.j-origin.j)*(point.i-origin.i) );
      }
   };
}
#endif//COLLISION_H
