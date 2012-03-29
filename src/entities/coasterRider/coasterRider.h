#ifndef COASTERRIDER_H
#define COASTERRIDER_H

#include "../../graphics/renderable/3DObject/sphere.h"
#include "../entity.h"
#include "../../types/vec3f.h"

#define MIN_RIDER_VELOCITY 0.1f
class coasterRider : public sphere, public entity, iRenderable
{
public:
	enum interpolationModes
	{
		e_nonLinear = 0,
		e_linear,
		e_linearPhysics
	};

	coasterRider():m_mode(e_linear),m_mu(1){};
	~coasterRider(){};
	bool interpolateToPoint(vec3f _nextPoint); //This interpolation is based on the points before it. If it gets there it returns true.
	void setInterpolationMode(interpolationModes _mode){m_mode = _mode;}
	void render(iRenderVisitor* _renderer){_renderer->visit(this);}
	vec3f getDirection(void){return m_dirVec;}

protected:
	vec3f m_lastPoint;
	vec3f m_currentPoint;
	vec3f m_dirVec;
	interpolationModes m_mode;
	float m_currentLength;
	float m_mu; //rate of change.
	float m_greatestHeight;
	float m_vel;
};

#endif//COASTERRIDER_H