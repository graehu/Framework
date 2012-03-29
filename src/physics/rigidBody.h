#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "../types/vec3f.h"
#include "../types/quaternion.h"
#include "../types/mat4x4f.h"
#include <math.h>

/// A cube with self contained physics simulation.
///
/// This class is responsible for maintaining and integrating its
/// physics state using an RK4 integrator. The nature of the
/// integrator requires that we structure this class in such a
/// way that all forces can be calculated from the current physics
/// state at any time. See Cube::integrate for details.

class rigidBody
{
public:

    /// Physics state.
    
    struct state
    {
        /// primary physics state
        vec3f position;                ///< the position of the cube center of mass in world coordinates (meters).
        vec3f momentum;                ///< the momentum of the cube in kilogram meters per second.
        quaternion orientation;         ///< the orientation of the cube represented by a unit quaternion.
        vec3f angularMomentum;         ///< angular momentum vector.

        // secondary state
        vec3f velocity;                ///< velocity in meters per second (calculated from momentum).
        quaternion spin;                ///< quaternion rate of change in orientation.
        vec3f angularVelocity;         ///< angular velocity (calculated from angularMomentum).
        //Mat4x4f bodyToWorld;             ///< body to world coordinates matrix.
        //Mat4x4f worldToBody;             ///< world to body coordinates matrix.

        /// constant state
        float size;                     ///< length of the cube sides in meters.
        float mass;                     ///< mass of the cube in kilograms.
        float inverseMass;              ///< inverse of the mass used to convert momentum to velocity.
        float inertiaTensor;            ///< inertia tensor of the cube (i have simplified it to a single value due to the mass properties a cube).
        float inverseInertiaTensor;     ///< inverse inertia tensor used to convert angular momentum to angular velocity.

        /// Recalculate secondary state values from primary values.

        void recalculate()
        {
            velocity = momentum * inverseMass;
            angularVelocity = angularMomentum * inverseInertiaTensor;
            orientation.normalise();
            spin = (quaternion(0, angularVelocity.i, angularVelocity.j, angularVelocity.k) * orientation)*0.5;
            //Mat4x4f translation;
            //translation.translate(position);
            //bodyToWorld = translation * orientation.matrix();
            //worldToBody = bodyToWorld.inverse();
        }
    };

    /// Default constructor.
	
	rigidBody()
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

    /// Update physics state.

    void update(float t, float dt)
    {
        previous = current;
        integrate(current, t, dt);
    }

    /// Render cube at interpolated state.
    /// Calculates interpolated state then renders cube at the interpolated 
	/// position and orientation using OpenGL.
	/// @param alpha interpolation alpha in [0,1]

    /*void render(float alpha = 1.0f)
	{	
		glPushMatrix();

		// interpolate state with alpha for smooth animation

		State state = interpolate(previous, current, alpha);
	
		// use position and orientation quaternion to build OpenGL body to world

		glTranslatef(state.position.x, state.position.y, state.position.z); 
		
		float angle;
		Mathematics::Vector axis;		
		state.orientation.angleAxis(angle, axis);
		glRotatef(angle/Mathematics::pi*180, axis.x, axis.y, axis.z);
		
        // render cube

		GLfloat color[] = { 1,1,1,1 };

		glMaterialfv(GL_FRONT, GL_AMBIENT, color);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
		
		glEnable(GL_LIGHTING);
		
        const float s = state.size * 0.5f;

        glBegin(GL_QUADS);
		
			glNormal3f(0,0,+1);
			glVertex3f(-s,-s,+s);
			glVertex3f(+s,-s,+s);
			glVertex3f(+s,+s,+s);
			glVertex3f(-s,+s,+s);

			glNormal3f(0,0,-1);
			glVertex3f(-s,-s,-s);
			glVertex3f(-s,+s,-s);
			glVertex3f(+s,+s,-s);
			glVertex3f(+s,-s,-s);
			
			glNormal3f(0,+1,0);
			glVertex3f(-s,+s,-s);
			glVertex3f(-s,+s,+s);
			glVertex3f(+s,+s,+s);
			glVertex3f(+s,+s,-s);
			
			glNormal3f(0,-1,0);
			glVertex3f(-s,-s,-s);
			glVertex3f(+s,-s,-s);
			glVertex3f(+s,-s,+s);
			glVertex3f(-s,-s,+s);

			glNormal3f(+1,0,0);
			glVertex3f(+s,-s,-s);
			glVertex3f(+s,+s,-s);
			glVertex3f(+s,+s,+s);
			glVertex3f(+s,-s,+s);
			
			glNormal3f(-1,0,0);
			glVertex3f(-s,-s,-s);
			glVertex3f(-s,-s,+s);
			glVertex3f(-s,+s,+s);
			glVertex3f(-s,+s,-s);
			
		glEnd();

		glDisable(GL_LIGHTING);

		glPopMatrix();
	}*/
	
private:

    /// Interpolate between two physics states.
	
	static state interpolate(const state &a, const state &b, float alpha)
	{
		state state = b;
		state.position = a.position*(1-alpha) + b.position*alpha;
		state.momentum = a.momentum*(1-alpha) + b.momentum*alpha;
		state.orientation = slerp(a.orientation, b.orientation, alpha);
		state.angularMomentum = a.angularMomentum*(1-alpha) + b.angularMomentum*alpha;
		state.recalculate();
		return state;
	}

	state previous;		///< previous physics state.
    state current;		///< current physics state.

    /// Derivative values for primary state.
    /// This structure stores all derivative values for primary state in Cube::State.
    /// For example velocity is the derivative of position, force is the derivative
    /// of momentum etc. Storing all derivatives in this structure makes it easy
    /// to implement the RK4 integrator cleanly because it needs to calculate the
    /// and store derivative values at several points each timestep.

	struct derivative
	{
		vec3f velocity;                ///< velocity is the derivative of position.
		vec3f force;                   ///< force in the derivative of momentum.
		quaternion spin;                ///< spin is the derivative of the orientation quaternion.
		vec3f torque;                  ///< torque is the derivative of angular momentum.
	};	

    /// Evaluate all derivative values for the physics state at time t.
    /// @param state the physics state of the cube.

	static derivative evaluate(const state &_state, float t)
	{
		derivative output;
		output.velocity = _state.velocity;
		output.spin = _state.spin;
		forces(_state, t, output.force, output.torque);
		return output;
	}
	
    /// Evaluate derivative values for the physics state at future time t+dt 
    /// using the specified set of derivatives to advance dt seconds from the 
    /// specified physics state.

	static derivative evaluate(state _state, float t, float dt, const derivative &_derivative)
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

    /// Integrate physics state forward by dt seconds.
    /// Uses an RK4 integrator to numerically integrate with error O(5).

	static void integrate(state &_state, float t, float dt)
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

    /// Calculate force and torque for physics state at time t.
    /// Due to the way that the RK4 integrator works we need to calculate
    /// force implicitly from state rather than explictly applying forces
    /// to the rigid body once per update. This is because the RK4 achieves
    /// its accuracy by detecting curvature in derivative values over the 
    /// timestep so we need our force values to supply the curvature.

	static void forces(const state &_state, float t, vec3f &force, vec3f &torque)
	{
		// attract towards origin

		force = -10 * _state.position;
	
		// sine force to add some randomness to the motion

		force.i += 10 * sin(t*0.9f + 0.5f);
		force.j += 11 * sin(t*0.5f + 0.4f);
		force.k += 12 * sin(t*0.7f + 0.9f);

		// sine torque to get some spinning action

		torque.i = 1.0f * sin(t*0.9f + 0.5f);
		torque.j = 1.1f * sin(t*0.5f + 0.4f);
		torque.k = 1.2f * sin(t*0.7f + 0.9f);

		// damping torque so we dont spin too fast

		torque -= 0.2f * _state.angularVelocity;
	}
};
#endif//RIGIDBODY_H