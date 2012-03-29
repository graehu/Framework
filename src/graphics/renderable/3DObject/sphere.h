#ifndef SPHERE_H
#define SPHERE_H

#include "../iRenderable.h"

class sphere
{
public:
	sphere() : m_radius(0.2f), m_vSegs(8), m_hSegs(8){}
	~sphere(){}

	void setRadius(float _radius){if(_radius > 0) m_radius = _radius;}
	float getRadius(void){return m_radius;}
	//Add accessors for segs.
protected:
	float m_radius;
	float m_vSegs;
	float m_hSegs;
private:
};

#endif//SPHERE_H