#ifndef BEZIERCURVE_H
#define BEZIERCURVE_H

#include <vector>
#include "vec3f.h"
#include "../graphics/renderable/iRenderable.h"

class bezierCurve : public iRenderable
{
public:
	bezierCurve(){m_selectedID = -1;}
	~bezierCurve(){}

	vec3f getPoint(int _pointID)
	{
		if(_pointID < m_points.size() &&  _pointID > -1)
			return m_points[_pointID];

		return vec3f(0,0,0);
	}

	unsigned int getNumPoints(void){return m_points.size();}
	void addPoint(vec3f _point){m_points.push_back(_point);}
	void render(iRenderVisitor* _renderer){_renderer->visit(this);}
	int getSelectedID(void){return m_selectedID;}
	void selectByEye(vec3f _position, vec3f _direction);
	vec3f getPointOnCurve(unsigned int _section, float _mu);
	vec3f& getSelectedPoint(void)
	{ 
		if(m_selectedID != -1)
			return m_points[m_selectedID];
	}
	void setSelectedID(int _ID)
	{//This should be doable in one line.
		if(_ID == -1)
		{ m_selectedID = -1;return;}
		if(_ID < m_points.size() && _ID >= 0) m_selectedID = _ID;
	}



protected:
private:

	std::vector<vec3f> m_points;
	int m_selectedID;


};

#endif//BEZIERCURVE_H