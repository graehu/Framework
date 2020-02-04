#include "game.h"
//#include "../graphics/renderable/sprite/sprite.h"
#include "../physics/rigidBody.h"
#include "../physics/polygon.h"
#include "../graphics/camera/camera.h"

#include <queue>
#include <cassert>

game::game() : m_name("physics")
{
  m_looping = true;
}

void game::init(void)
{
  m_window = window::windowFactory();
  m_window->init(512, 512, m_name);
  m_graphics = graphics::graphicsFactory();
  m_graphics->init();
  m_input = input::inputFactory();
  m_input->init();
  m_network = new net::network(0xf00d, 1000);
  for(int i = 0; m_network->init(i==0?1:0, 8000+i); i++) { }
}

void game::run(void)
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

  polygon player, floor, wall1, wall2;
  std::vector<rigidBody> test;

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
		  test.push_back(rigidBody());
		  test[test.size()-1].m_vertices.push_back(vec3f(-0.75f,-0.75f));
		  test[test.size()-1].m_vertices.push_back(vec3f(-0.75f,0.75f));
		  test[test.size()-1].m_vertices.push_back(vec3f(0.75f,0.75f));
		  test[test.size()-1].m_vertices.push_back(vec3f(0.75f,-0.75f));
		  spawning = false;
	  }else if(!m_input->isKeyPressed(input::e_respawn)) spawning = false;

	  floor.render(m_graphics->getRenderer());
	  wall1.render(m_graphics->getRenderer());
	  wall2.render(m_graphics->getRenderer());
	  for(int i = 0; i < test.size(); i++)
	  {
		  test[i].collideSAT(&wall2);
		  test[i].collideSAT(&wall1);
		  test[i].collideSAT(&floor);
		  test[i].collideSAT(&player);
	  }
	  for(int i = 0; i < test.size(); i++)
	  {
		  for(int ii = 0; ii < test.size(); ii++)
		  {
			  if(i == ii) continue;
			  test[i].collideSAT(&test[ii]);
		  }
	  }
	  m_network->recievePacket();
	  m_network->update(dt);
	  m_graphics->render();
	 // Sleep(30);
	}
}



game::~game()
{
}

///find a better place to do hit detection you foo'

/*
  bool game::hitTest(square sOne, square sTwo)
  {
  int x1 = *(sOne.getPosX());
  int y1 = *(sOne.getPosY());
  int height1 = *(sOne.getHeight());
  int width1 = *(sOne.getWidth());

  int x2 = *(sTwo.getPosX());
  int y2 = *(sTwo.getPosY());
  int height2 = *(sTwo.getHeight());
  int width2 = *(sTwo.getWidth());

  if (((x1 + width1 > x2) && (x1 < (x2 + width2))) && ((y1 + height1 > y2) && (y1 < (y2 + height2))))
  {
  return true;
  }
  else
  {
  return false;
  }
  }


*/
