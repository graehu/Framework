#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <math.h>
#include <vector>
#include "physics_state.h"
#include "collision.h"
#include "../types/transform.h"

namespace physics
{
   namespace collider
   {
       class collider;  
   }
   class rigid_body
   {
     public:

      /// Physics state.
      rigid_body();
      vec3f get_position(void) { return m_current_state.position; }
      void set_position(vec3f _position) { m_current_state.position = _position; }
      void set_mass(float _mass)
      {
	 // this doesn't really seem to function correctly in the integrator.
	 m_current_state.mass = _mass;
	 // m_current_state.inverseMass = 1.0f / m_current_state.mass;
	 // m_current_state.inertiaTensor = m_current_state.mass * m_current_state.size * m_current_state.size * 1.0f / 6.0f;
	 // m_current_state.inverseInertiaTensor = 1.0f / m_current_state.inertiaTensor;
      }
      void add_collision(collision _collision);
      void set_debug_name(const char* _name) { m_debug_name = _name; }
      void set_debug(bool enabled) { m_debug = enabled; }
      const char* get_debug_name() { return m_debug_name; }
      quaternion get_orientation(void) { return m_current_state.orientation; }
      core::transform get_transform();

      void render(class iRenderVisitor* _renderer);
      ///Update physics state.
      void update(float t, float dt);
      ///Apply a force.
      void apply_force(vec3f _force) { m_current_state.forces.push_back(_force); }
      void apply_impulse(vec3f _force)
      {
	 m_current_state.forces.push_back(_force);
	 m_current_state.momentum += _force;
      }
      //assumes ownership of passed collider.
      void add_collider(collider::collider* _collider);
      //resolve stored up collisions
      void resolve_collisions();
     protected:
      // #todo: support more colliders than one.
      collider::collider* m_collider = nullptr;
      state m_previous_state;
      state m_current_state;
      std::vector<collision> m_collisions;
      const char* m_debug_name;
      bool m_debug;
      
     private:

      void integrate(state &_state, float t, float dt);
      derivative evaluate(const state &_state, float t);
      derivative evaluate(state _state, float t, float dt, const derivative &_derivative);
      void forces(const state &_state, float t, vec3f &force, vec3f &torque);

   };
}
#endif//RIGID_BODY_H
