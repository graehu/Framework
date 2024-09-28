#include "net_physics_sample.h"
#include "../../physics/rigid_body.h"
#include "../../physics/colliders/box.h"
#include "../../graphics/camera/camera.h"
#include "../../physics/collision_manager.h"
// #include "../../physics/colliders/polygon.h"
#include "../../utils/log/log.h"
#include "../../utils/params.h"
#include <queue>
#include <cassert>
#include <chrono>
#include <thread>

net_physics_sample::net_physics_sample() : m_name("physics")
{
   m_looping = true;
}
application* application::factory()
{
   return new net_physics_sample();
}
void net_physics_sample::init(void)
{
   m_window = window::windowFactory();
   m_window->init(512, 512, m_name);
   m_graphics = graphics::graphicsFactory();
   m_graphics->init();
   m_input = input::inputFactory();
   m_input->init();
}

void net_physics_sample::run(void)
{
   fw::log::topics::add("physics_sample");
   fw::log::scope("physics_sample");
   fw::log::topics::set_level("physics_sample", fw::log::e_info);
   init();
   
   camera game_cam;
   game_cam.m_view.perspective(60.0f, (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 100.f);
   mat4x4f translation;
   translation.translate(0, 0, -32);
   game_cam.m_view = translation*game_cam.m_view;
   game_cam.render(m_graphics->getRenderer());

   physics::collider::polygon floor, wall1, wall2;
   physics::collider::polygon wedge;

   std::vector<physics::rigid_body> test;
   
   floor.m_vertices.push_back(vec3f(-19.0f,-19.0f));
   floor.m_vertices.push_back(vec3f(-19.0f,-14.0f));
   floor.m_vertices.push_back(vec3f(19.0f,-14.0f));
   floor.m_vertices.push_back(vec3f(19.0f,-19.0f));
   floor.recalculate();
      
   wall1.m_vertices.push_back(vec3f(-14.0f, -19.0f));
   wall1.m_vertices.push_back(vec3f(-19.0f, -19.0f));
   wall1.m_vertices.push_back(vec3f(-19.0f, 14.0f));
   wall1.m_vertices.push_back(vec3f(-14.0f, 14.0f));
   wall1.recalculate();
   
   wall2.m_vertices.push_back(vec3f(14.0f, -19.0f));
   wall2.m_vertices.push_back(vec3f(19.0f, -19.0f));
   wall2.m_vertices.push_back(vec3f(19.0f, 14.0f));
   wall2.m_vertices.push_back(vec3f(14.0f, 14.0f));
   wall2.recalculate();

   wedge.m_vertices.push_back({0.0f, 0.0f});
   wedge.m_vertices.push_back({1.0f, -1.0f});
   wedge.m_vertices.push_back({-1.0f, -1.0f});
   wedge.m_position = {0, -5, 0};
   wedge.recalculate();
   
   {
      for(int i = -6; i < 7; i++)
      {
	 for(int ii = -6; ii < 7; ii++)
	 {
	    auto col = new physics::collider::box(0.5f);
	    test.push_back(physics::rigid_body());
	    test[test.size()-1].add_collider(col);
	    test[test.size()-1].set_position({float(i*2), -float(ii*2), 0});
	    test[test.size()-1].set_mass(2);
	 }
      }
      fw::log::info("created {} bodies", test.size());
   }
   physics::rigid_body player;
   player.set_debug(false);
   player.set_debug_name("player");
   {
      auto col = new physics::collider::box(1.0f);
      player.add_collider(col);
   }
   player.set_position({0, 10, 0});
   player.set_mass(20);

   float time = 0;
   bool spawning = false;
   std::chrono::high_resolution_clock clock;
   auto clock_before = clock.now();
   while(m_looping)
   {
      auto clock_now = clock.now();
      float dt = std::chrono::duration<float>(clock_now-clock_before).count();
      clock_before = clock_now;
      // fw::log::debug("{}",dt);
      fw::log::timer loop("loop");
      if(m_input->update() || m_input->isKeyPressed(input::e_quit)) m_looping = false;
      player.update(time, dt);
      {
	 // fw::log::timer("update");
	 for(unsigned int i = 0; i < test.size(); i++)
	 {
	    test[i].render(m_graphics->getRenderer());  
	    test[i].update(time, dt);
	 }
      }
      time += dt;

      player.render(m_graphics->getRenderer());
      
      if(m_input->isKeyPressed(input::e_left))
	 player.apply_impulse({-.2,0,0});
      
      if(m_input->isKeyPressed(input::e_right))
	 player.apply_impulse({0.2,0,0});

      if(m_input->isKeyPressed(input::e_up))
	 player.apply_impulse({0,.5,0});

      if(m_input->isKeyPressed(input::e_down))
	 player.apply_impulse({0,-.5,0});

      if(m_input->isKeyPressed(input::e_respawn) && spawning == false)
      {
	 test.push_back(physics::rigid_body());
	 test[test.size()-1].add_collider(new physics::collider::box(0.75f));
	 spawning = false;
      }
      else if(!m_input->isKeyPressed(input::e_respawn)) spawning = false;

      floor.render(m_graphics->getRenderer());
      wall1.render(m_graphics->getRenderer());
      wall2.render(m_graphics->getRenderer());
      wedge.render(m_graphics->getRenderer());
      {
	 physics::collision_manager::update();
      }

      m_graphics->render();
   }
}
