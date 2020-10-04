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
      void add_collision(collision _collision);
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

     private:

      void integrate(state &_state, float t, float dt);
      derivative evaluate(const state &_state, float t);
      derivative evaluate(state _state, float t, float dt, const derivative &_derivative);
      void forces(const state &_state, float t, vec3f &force, vec3f &torque);
   };
}
#endif//RIGID_BODY_H
