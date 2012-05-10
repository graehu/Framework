#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "polygon.h"
#include <math.h>
#include "physicsState.h"

//struct state;

class rigidBody : public polygon
{
public:

    /// Physics state.
	rigidBody();
	~rigidBody(){};
	vec3f getPos(void){return current.position;}
	void setPos(vec3f _pos){current.position = _pos;}
	quaternion getOrientation(void) {return current.orientation;}

	vec3f collideSAT(rigidBody* _body);
	vec3f collideSAT(polygon* _poly);

	void render(iRenderVisitor* _renderer);
    ///Update physics state.
    void update(float t, float dt);
	///Apply a force.
	void applyForce(vec3f _force){m_forces.push_back(_force);}

protected:

	state previous;
    state current;
	std::vector<vec3f> m_forces;

private:

	void integrate(state &_state, float t, float dt);
	derivative evaluate(const state &_state, float t);
	derivative evaluate(state _state, float t, float dt, const derivative &_derivative);
	void forces(const state &_state, float t, vec3f &force, vec3f &torque);
};
#endif//RIGIDBODY_H