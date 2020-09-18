#ifndef PHYSICS_STATE_H
#define PHYSICS_STATE_H

#include "../types/vec3f.h"
#include "../types/quaternion.h"
#include "../types/mat4x4f.h"
#include <vector>

namespace physics
{
   struct state
   {
      /// primary physics state
      vec3f position;                // the position of the cube center of mass in world coordinates (meters).
      vec3f momentum;                // the momentum of the cube in kilogram meters per second.
      quaternion orientation;        // the orientation of the cube represented by a unit quaternion.
      vec3f angularMomentum;         // angular momentum vector.

      // secondary state
      vec3f velocity;                // velocity in meters per second (calculated from momentum).
      quaternion spin;               // quaternion rate of change in orientation.
      vec3f angularVelocity;         // angular velocity (calculated from angularMomentum).

      /// constant state
      float size;                    // length of the cube sides in meters.
      float mass;                    // mass of the cube in kilograms.
      float inverseMass;             // inverse of the mass used to convert momentum to velocity.
      float inertiaTensor;           // inertia tensor of the cube (i have simplified it to a single value due to the mass properties a cube).
      float inverseInertiaTensor;    // inverse inertia tensor used to convert angular momentum to angular velocity.

      /// Recalculate secondary state values from primary values.
      std::vector<vec3f> forces;
      void recalculate()
      {
	 velocity = momentum * inverseMass;
	 angularVelocity = angularMomentum * inverseInertiaTensor;
	 orientation.normalise();
	 spin = (quaternion(0, angularVelocity.i, angularVelocity.j, angularVelocity.k) * orientation)*0.5;
      }
   };
   
   struct derivative
   {
      vec3f velocity;                // velocity is the derivative of position.
      vec3f force;                   // force in the derivative of momentum.

      quaternion spin;               // spin is the derivative of the orientation quaternion.
      vec3f torque;                  // torque is the derivative of angular momentum.
   };	
}

#endif//PHYSICS_STATE_H
