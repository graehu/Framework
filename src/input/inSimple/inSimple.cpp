#include "inSimple.h"
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <ostream>
#include <thread>
#include <chrono>
#include <iostream>
#include <stdint.h>

input::~input()
{
}
inSimple::inSimple()
{
  m_thread = new std::thread(&inSimple::pollInput, this);
  m_dying = false;
}
inSimple::~inSimple()
{
  m_dying = true;
  m_thread->join();
  delete m_thread;
}
static bool die = false;
void inSimple::pollInput()
{
  bool delay_death = true;
  while(delay_death)
  {
    if(!m_dying && !die)
    {
      char input = 0;
      std::cin >> input;
      if(input > 0)
      {
	if(input == 'q')
	  {
	    die = true;
	  }
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    if(m_dying)
    {
      delay_death = false;
    }
  }
}
int inSimple::init(void)
{
  return 0;
}
bool inSimple::update(void)
{
  return die;
}
bool inSimple::isKeyPressed(keys _key)
{
  return false;
}
bool inSimple::isMouseClicked(mouseButtons _button)
{
  return false;
}
void inSimple::mouseDelta(float& _dx, float& _dy)
{
  return;
}

input* input::inputFactory(void)
{
  return (input*)new inSimple();
}
