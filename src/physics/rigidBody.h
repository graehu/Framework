#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "../types/vec3f.h"
#include "../types/quaternion.h"
#include "../types/mat4x4f.h"
#include "polygon.h"
#include <math.h>

/// A cube with self contained physics simulation.
///
/// This class is responsible for maintaining and integrating its
/// physics state using an RK4 integrator. The nature of the
/// integrator requires that we structure this class in such a
/// way that all forces can be calculated from the current physics
/// state at any time. See Cube::integrate for details.

class rigidBody : public polygon
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
        }
    };
	vec3f getPos(void){return current.position;}
	quaternion getOrientation(void) {return current.orientation;}

	vec3f collideSAT(polygon* _poly);
	void render(iRenderVisitor* _renderer);

    /// Default constructor.
	
	rigidBody();

    /// Update physics state.

    void update(float t, float dt);

private:

    /// Interpolate between two physics states.
	
	state interpolate(const state &a, const state &b, float alpha);

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

	derivative evaluate(const state &_state, float t);
	
    /// Evaluate derivative values for the physics state at future time t+dt 
    /// using the specified set of derivatives to advance dt seconds from the 
    /// specified physics state.

	derivative evaluate(state _state, float t, float dt, const derivative &_derivative);
    /// Integrate physics state forward by dt seconds.
    /// Uses an RK4 integrator to numerically integrate with error O(5).

	void integrate(state &_state, float t, float dt);	

    /// Calculate force and torque for physics state at time t.
    /// Due to the way that the RK4 integrator works we need to calculate
    /// force implicitly from state rather than explictly applying forces
    /// to the rigid body once per update. This is because the RK4 achieves
    /// its accuracy by detecting curvature in derivative values over the 
    /// timestep so we need our force values to supply the curvature.

	void forces(const state &_state, float t, vec3f &force, vec3f &torque);
};
#endif//RIGIDBODY_H