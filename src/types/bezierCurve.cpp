#include "bezierCurve.h"
bool raySphereIntersect(const vec3f &p, const vec3f &d, double r);//, double  &i1, double &i2);


void bezierCurve::selectByEye(vec3f _position, vec3f _direction)
{
	for(int i = 0; i < (getNumPoints()/3)+1; i++)
	{
	  if(raySphereIntersect(getPoint(i*3) - _position, _direction, 0.2))
	  {
		  printf("selected:%i\n",i*3);
		  setSelectedID(i*3);
		  return;
	  }
	}
	if(getSelectedID() == 0)
	{
	  if(raySphereIntersect(getPoint(1) - _position,_direction, 0.2))
		  setSelectedID(1);
	  else {setSelectedID(-1);return;}
	  
	}
	else if(getSelectedID() == getNumPoints()-1)
	{
	  if(raySphereIntersect(getPoint(getNumPoints()-2) - _position,_direction, 0.2))
		  setSelectedID(getNumPoints()-2);
	  else {setSelectedID(-1);return;}
	}
	else
	{
	  for(int i = 0; i < 2; i++)
	  {
		  if(raySphereIntersect(getPoint(getSelectedID()-1+(i*2)) - _position,_direction, 0.2))
		  {
			  setSelectedID(getSelectedID()-1+(i*2));
			  return;
		  }
	  }
	  setSelectedID(-1);
	}
}

bool raySphereIntersect(const vec3f &_position, const vec3f &_direction, double _radius)//, double  &i1, double &i2) //for intersection points.
{
	double det,b;
	b = -DotProduct3(_position,_direction);
	det = b*b - DotProduct3(_position,_position) + _radius*_radius;
	if (det<0){
		return false;
	}
	det= sqrt(det);
	double i1= b - det;
	double i2= b + det;
	// intersecting with ray?
	if(i2<0) return false;
	if(i1<0)i1=0;
	return true;
};

vec3f bezierCurve::getPointOnCurve(unsigned int _section, float _mu)
{
	//PUT CHECKS HERE

	vec3f a,b,c,p;
	//start - mid1
	c.i = 3 * (m_points[(_section*3)+1].i - m_points[(_section*3)].i);
	c.j = 3 * (m_points[(_section*3)+1].j - m_points[(_section*3)].j);
	c.k = 3 * (m_points[(_section*3)+1].k - m_points[(_section*3)].k);
	//mid2 - mid1
	b.i = 3 * (m_points[(_section*3)+2].i - m_points[(_section*3)+1].i) - c.i;
	b.j = 3 * (m_points[(_section*3)+2].j - m_points[(_section*3)+1].j) - c.j;
	b.k = 3 * (m_points[(_section*3)+2].k - m_points[(_section*3)+1].k) - c.k;
	//start - end
	a.i = m_points[(_section*3)+3].i - m_points[(_section*3)].i - c.i - b.i;
	a.j = m_points[(_section*3)+3].j - m_points[(_section*3)].j - c.j - b.j;
	a.k = m_points[(_section*3)+3].k - m_points[(_section*3)].k - c.k - b.k;

	p.i = a.i * _mu * _mu * _mu + b.i * _mu * _mu + c.i * _mu + m_points[(_section*3)].i;
	p.j = a.j * _mu * _mu * _mu + b.j * _mu * _mu + c.j * _mu + m_points[(_section*3)].j;
	p.k = a.k * _mu * _mu * _mu + b.k * _mu * _mu + c.k * _mu + m_points[(_section*3)].k;

	return p;
}