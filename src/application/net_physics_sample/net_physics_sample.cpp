#include "net_physics_sample.h"
#include "../../physics/rigid_body.h"
#include "../../physics/colliders/box.h"
#include "../../graphics/camera/camera.h"
#include "../../physics/collision_manager.h"

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
   // m_network = new net::network(0xf00d, 1000);
   // for(int i = 0; m_network->init(i==0?1:0, 8000+i); i++) { }
}

void net_physics_sample::mf_run(void)
{
   init();

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

   physics::collider::polygon player, floor, wall1, wall2;
   std::vector<physics::rigid_body> test;

   player.m_vertices.push_back(vec3f(-1.0f,-1.0f));
   player.m_vertices.push_back(vec3f(-1.0f, 1.0f));
   player.m_vertices.push_back(vec3f(1.0f,1.0f));
   player.m_vertices.push_back(vec3f(1.0f,-1.0f));

   floor.m_vertices.push_back(vec3f(-19.0f,-19.0f));
   floor.m_vertices.push_back(vec3f(-19.0f,-14.0f));
   floor.m_vertices.push_back(vec3f(19.0f,-14.0f));
   floor.m_vertices.push_back(vec3f(19.0f,-19.0f));

   wall1.m_vertices.push_back(vec3f(-14.0f, -19.0f));
   wall1.m_vertices.push_back(vec3f(-19.0f, -19.0f));
   wall1.m_vertices.push_back(vec3f(-19.0f, 14.0f));
   wall1.m_vertices.push_back(vec3f(-14.0f, 14.0f));

   wall2.m_vertices.push_back(vec3f(14.0f, -19.0f));
   wall2.m_vertices.push_back(vec3f(19.0f, -19.0f));
   wall2.m_vertices.push_back(vec3f(19.0f, 14.0f));
   wall2.m_vertices.push_back(vec3f(14.0f, 14.0f));


   float time = 0;
   float dt = 1.0f/30.0f;
   bool spawning = false;

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
	 for(unsigned int i = 0; i < player.m_vertices.size(); i++)
	    player.m_vertices[i].i -= 0.1f;

      if(m_input->isKeyPressed(input::e_right))
	 for(unsigned int i = 0; i < player.m_vertices.size(); i++)
	    player.m_vertices[i].i += 0.1f;

      if(m_input->isKeyPressed(input::e_up))
	 for(unsigned int i = 0; i < player.m_vertices.size(); i++)
	    player.m_vertices[i].j += 0.1f;

      if(m_input->isKeyPressed(input::e_down))
	 for(unsigned int i = 0; i < player.m_vertices.size(); i++)
	    player.m_vertices[i].j -= 0.1f;

      if(m_input->isKeyPressed(input::e_respawn) && spawning == false)
      {
	 test.push_back(physics::rigid_body());
	 test[test.size()-1].add_collider(new physics::collider::box(0.75f));
	 spawning = false;
      }else if(!m_input->isKeyPressed(input::e_respawn)) spawning = false;

      floor.render(m_graphics->getRenderer());
      wall1.render(m_graphics->getRenderer());
      wall2.render(m_graphics->getRenderer());

      physics::collision_manager::update();
      
      // m_network->recievePacket();
      // m_network->update(dt);
      m_graphics->render();
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
   }
}



net_physics_sample::~net_physics_sample()
{
}
