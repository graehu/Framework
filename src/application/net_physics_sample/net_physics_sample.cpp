#include "net_physics_sample.h"
#include "../../physics/rigid_body.h"
#include "../../physics/colliders/box.h"
#include "../../graphics/camera/camera.h"
#include "../../physics/collision_manager.h"
// #include "../../utils/log/log.h"
// #include "../../utils/params.h"
#include <queue>
#include <cassert>
#include <chrono>
#include <thread>

net_physics_sample::net_physics_sample() : m_name("physics")
{
   m_looping = true;
}
application* application::mf_factory()
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

void net_physics_sample::mf_run(void)
{
   init();
   // commandline::parse();
   // auto topic = log::scope("poop");
   //setup the camera and render once. (no movement in this game.)
   camera game_cam;
   game_cam.m_view.perspective(60.0f, (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 100.f);
   mat4x4f translation;
   translation.translate(0, 0, -32);
   game_cam.m_view = translation*game_cam.m_view;
   game_cam.render(m_graphics->getRenderer());

   std::queue<char> inputs;
   for(int i = 0; i < 6; i++)
      inputs.push(0);

   physics::collider::polygon floor, wall1, wall2, wedge;
   physics::collider::box player;
   player.m_position = {0, 5, 0};
   player.recalculate();
   std::vector<physics::rigid_body> test;
   // player.add_collider(new physics::collider::box());
   
   
   // player.recalculate();
   
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
      auto col = new physics::collider::box(0.75f);
      test.push_back(physics::rigid_body());
      test[test.size()-1].add_collider(col);
      test[test.size()-1].set_position({-1, 0, 0});
      
      col = new physics::collider::box(0.75f);
      test.push_back(physics::rigid_body());
      test[test.size()-1].add_collider(col);
      test[test.size()-1].set_position({1, 0, 0});

      col = new physics::collider::box(0.75f);
      test.push_back(physics::rigid_body());
      test[test.size()-1].add_collider(col);
      test[test.size()-1].set_position({1, 1, 0});
      
      col = new physics::collider::box(0.75f);
      test.push_back(physics::rigid_body());
      test[test.size()-1].add_collider(col);
      test[test.size()-1].set_position({-1, 1, 0});
      // test.push_back(physics::rigid_body());
      // test[test.size()-1].add_collider(new physics::collider::box(0.75f));
   }

   float time = 0;
   float last_spawn = 10.0f;
   bool spawning = false;
   std::chrono::high_resolution_clock clock;
   auto clock_before = clock.now();
   float dt = 1.0f/60.0f;
   while(m_looping)
   {
      if(m_input->update()) m_looping = false;
      for(int i = 0; i < test.size(); i++)
      {
	 test[i].render(m_graphics->getRenderer());
	 test[i].update(time, dt);
      } 
      
      time += dt;

      player.render(m_graphics->getRenderer());

      if(m_input->isKeyPressed(input::e_left))
      	    player.m_position.i -= 0.1f;

      if(m_input->isKeyPressed(input::e_right))
      	    player.m_position.i += 0.1f;

      if(m_input->isKeyPressed(input::e_up))
      	    player.m_position.j += 0.1f;

      if(m_input->isKeyPressed(input::e_down))
      	    player.m_position.j -= 0.1f;

      if(m_input->isKeyPressed(input::e_respawn) && spawning == false)
      {
	 test.push_back(physics::rigid_body());
	 test[test.size()-1].add_collider(new physics::collider::box(0.75f));
	 spawning = false;
      }else if(!m_input->isKeyPressed(input::e_respawn)) spawning = false;

      floor.render(m_graphics->getRenderer());
      wall1.render(m_graphics->getRenderer());
      wall2.render(m_graphics->getRenderer());
      wedge.render(m_graphics->getRenderer());

      physics::collision_manager::update();
      

      m_graphics->render();
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
      // dt = std::chrono::duration<float>(clock.now()-clock_before).count();///1000.0f;
      clock_before = clock.now();
      // printf("%f\n", dt);
   }
}



net_physics_sample::~net_physics_sample()
{
}
