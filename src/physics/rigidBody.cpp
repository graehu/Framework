#include "rigidBody.h"

rigidBody::rigidBody()
{
	current.size = 1;
	current.mass = 1;
	current.inverseMass = 1.0f / current.mass;
	current.position = vec3f(2,0,0);
	current.momentum = vec3f(0,0,-10);
	current.orientation.identity();
	current.angularMomentum = vec3f(0,0,0);
	current.inertiaTensor = current.mass * current.size * current.size * 1.0f / 6.0f;
	current.inverseInertiaTensor = 1.0f / current.inertiaTensor;
	current.recalculate();
	previous = current;
}

//TODO: this needs to be done differently.
//      the physics engine should be the thing
//      updating bodies and resolving collisions.
void rigidBody::update(float t, float dt)
{
	previous = current;
	integrate(current, t, dt);
	m_forces.clear();
	//if physics are 2d. do this:
	current.position.k = 0;
	current.orientation.i = 0;
	current.orientation.j = 0;
}
void rigidBody::integrate(state &_state, float t, float dt)
{
	derivative a = evaluate(_state, t);
	derivative b = evaluate(_state, t, dt*0.5f, a);
	derivative c = evaluate(_state, t, dt*0.5f, b);
	derivative d = evaluate(_state, t, dt, c);
	
	_state.position += 1.0f/6.0f * dt * (a.velocity + 2.0f*(b.velocity + c.velocity) + d.velocity);
	_state.momentum += 1.0f/6.0f * dt * (a.force + 2.0f*(b.force + c.force) + d.force);

	_state.orientation += (a.spin + (b.spin + c.spin)*2.0 + d.spin)*(1.0f/6.0f * dt);

	_state.angularMomentum += 1.0f/6.0f * dt * (a.torque + 2.0f*(b.torque + c.torque) + d.torque);

	_state.recalculate();
}	

derivative rigidBody::evaluate(const state &_state, float t)
{
	derivative output;
	output.velocity = _state.velocity;
	output.spin = _state.spin;
	forces(_state, t, output.force, output.torque);
	return output;
}

derivative rigidBody::evaluate(state _state, float t, float dt, const derivative &_derivative)
{
	_state.position += _derivative.velocity * dt;
	_state.momentum += _derivative.force * dt;
	_state.orientation +=  _derivative.spin * dt;
	_state.angularMomentum += _derivative.torque * dt;
	_state.recalculate();
	
	derivative output;
	output.velocity = _state.velocity;
	output.spin = _state.spin;
	forces(_state, t+dt, output.force, output.torque);
	return output;
}



void rigidBody::forces(const state &_state, float t, vec3f &force, vec3f &torque)
{
	// attract towards origin
	force.j = -9.8;// * _state.position.j;

	for(int i = 0; i < m_forces.size(); i++)
		force = force + m_forces[i];



	// sine force to add some randomness to the motion
	
	//force.i += 10;// * sin(t*0.9f + 0.5f);
	//force.j += 11;// * sin(t*0.5f + 0.4f);
	//force.k += 12;// * sin(t*0.7f + 0.9f);

	// sine torque to get some spinning action

	//torque.i = 1.0f;// * sin(t*0.9f + 0.5f);
	//torque.j = 1.1f;// * sin(t*0.5f + 0.4f);
	//torque.k = 1.2f;// * sin(t*0.7f + 0.9f);

	// damping torque so we dont spin too fast

	//torque -= 0.2f * _state.angularVelocity;
}



void rigidBody::render(iRenderVisitor* _renderer)
{
	std::vector<vec3f> temp = m_vertices;
	mat4x4f orient;
	current.orientation.createMatrix(&orient);
	for(unsigned int i = 0; i < m_vertices.size(); i++)
		m_vertices[i] = (orient)*m_vertices[i] + current.position;
	_renderer->visit(this);
	m_vertices = temp;
}


vec3f rigidBody::collideSAT(polygon* _poly)
{
	//Do interesting shaz here.
	std::vector<vec3f> temp = m_vertices;
	for(unsigned int i = 0; i < m_vertices.size(); i++)
		m_vertices[i] = m_vertices[i] + current.position;
	vec3f MTV = polygon::collideSAT(_poly);

	m_forces.push_back(MTV*200);//*1000

	current.position += MTV;
	m_vertices = temp;
	return MTV;
};

vec3f rigidBody::collideSAT(rigidBody* _body)
{
	//Do interesting shaz here.
        //This moves the body's verts so they can be collided in world space
        //because i didn't want to store an offset with the polygon verts?
	for(unsigned int i = 0; i < m_vertices.size(); i++)
		m_vertices[i] = m_vertices[i] + current.position;
	

	for(unsigned int i = 0; i < _body->m_vertices.size(); i++)
		_body->m_vertices[i] = _body->m_vertices[i]+_body->getPos();

	vec3f MTV = polygon::collideSAT((polygon*)_body);

	for(unsigned int i = 0; i < m_vertices.size(); i++)
		m_vertices[i] = m_vertices[i] - current.position;

	for(unsigned int i = 0; i < _body->m_vertices.size(); i++)
		_body->m_vertices[i] = _body->m_vertices[i] - _body->getPos();

	m_forces.push_back(MTV*200);//*1000

	current.position += MTV*0.75f;
	return MTV;
};

///this function doesn't even get called...
state interpolate(const state &a, const state &b, float alpha)
{
	state state = b;
	state.position = a.position*(1-alpha) + b.position*alpha;
	state.momentum = a.momentum*(1-alpha) + b.momentum*alpha;
	state.orientation = slerp(a.orientation, b.orientation, alpha);
	state.angularMomentum = a.angularMomentum*(1-alpha) + b.angularMomentum*alpha;
	state.recalculate();
	return state;
}
