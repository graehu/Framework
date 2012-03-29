#include "fuzzyMembership.h"
#include "fuzzyRule.h"
#include "fuzzySet.h"
#include <cstdio>
#include <cstdlib>
bool success = true;
#include <iostream>
using namespace std;

fuzzySet distanceSet("distance");
fuzzySet speedSet("speed");
fuzzySet steerSet("steer");

std::vector<fuzzySet> fuzzySets;
std::vector<fuzzyRule> fuzzyRules;

char* distances[] = {"LargeDistL", "SmallDistL", "NoDist", "SmallDistR", "LargeDistR"};
char* speeds[] = {"FastSpeedL", "SlowSpeedL", "NoSpeed", "SlowSpeedR", "FastSpeedR"};
char* steerDegrees[] = {"HardTurnL", "SoftTurnL", "NoTurn", "SoftTurnR", "HardTurnR"};

void init()
{


	///input1.
	distanceSet.m_memberships.push_back(fuzzyMembership(distances[0], vec3f(-1.5,-0.5), fuzzyMembership::e_triangle));
	distanceSet.m_memberships.push_back(fuzzyMembership(distances[1], vec3f(-0.75,0.0), fuzzyMembership::e_triangle));
	distanceSet.m_memberships.push_back(fuzzyMembership(distances[2], vec3f(-0.25,0.25), fuzzyMembership::e_triangle));
	distanceSet.m_memberships.push_back(fuzzyMembership(distances[3], vec3f(0.0,0.75), fuzzyMembership::e_triangle));
	distanceSet.m_memberships.push_back(fuzzyMembership(distances[4], vec3f(0.5,1.5), fuzzyMembership::e_triangle));
	///input2.
	speedSet.m_memberships.push_back(fuzzyMembership(speeds[0], vec3f(-1.5, -0.5), fuzzyMembership::e_triangle));
	speedSet.m_memberships.push_back(fuzzyMembership(speeds[1], vec3f(-0.75, 0.0), fuzzyMembership::e_triangle));
	speedSet.m_memberships.push_back(fuzzyMembership(speeds[2], vec3f(-0.25, 0.25),fuzzyMembership::e_triangle));
	speedSet.m_memberships.push_back(fuzzyMembership(speeds[3], vec3f(0.0, 0.75), fuzzyMembership::e_triangle));
	speedSet.m_memberships.push_back(fuzzyMembership(speeds[4], vec3f(0.5, 1.5), fuzzyMembership::e_triangle));
	///output.
	steerSet.m_memberships.push_back(fuzzyMembership(steerDegrees[0], vec3f(-1.5,-0.5), fuzzyMembership::e_triangle));
	steerSet.m_memberships.push_back(fuzzyMembership(steerDegrees[1], vec3f(-0.75, 0.0), fuzzyMembership::e_triangle));
	steerSet.m_memberships.push_back(fuzzyMembership(steerDegrees[2], vec3f(-0.25, 0.25), fuzzyMembership::e_triangle));
	steerSet.m_memberships.push_back(fuzzyMembership(steerDegrees[3], vec3f(0.0, 0.75), fuzzyMembership::e_triangle));
	steerSet.m_memberships.push_back(fuzzyMembership(steerDegrees[4], vec3f(0.5, 1.5), fuzzyMembership::e_triangle));
	///sets.
	fuzzySets.push_back(distanceSet);
	fuzzySets.push_back(speedSet);
	fuzzySets.push_back(steerSet);
	///rules.
	fuzzyRules.push_back(fuzzyRule("HardLeft", distances[4], speeds[4], steerDegrees[0], fuzzyRule::e_OR));
	fuzzyRules.push_back(fuzzyRule("SoftLeft", distances[3], speeds[3], steerDegrees[1], fuzzyRule::e_OR));
	fuzzyRules.push_back(fuzzyRule("Straight", distances[2], speeds[2], steerDegrees[2], fuzzyRule::e_OR));
	fuzzyRules.push_back(fuzzyRule("HardRight", distances[1], speeds[1], steerDegrees[3], fuzzyRule::e_OR));
	fuzzyRules.push_back(fuzzyRule("SoftRight", distances[0], speeds[0], steerDegrees[4], fuzzyRule::e_OR));
}


float test(float distance, float rot)
{ 
	fuzzySets[0].update(distance);
	fuzzySets[1].update(rot);
	for(unsigned int i = 0; i < fuzzyRules.size(); i++)
	{
		fuzzyRules[i].evaluateSets(&fuzzySets);
	}
	printf("turning amount %s: %f\n",fuzzySets[2].m_biggest->m_name,fuzzySets[2].defuzzify(fuzzySet::e_mean));

	return 0;
}
void closedTest()
{
  cout << "Enter starting distance" << endl;
  float distance;
  cin >> distance;
  cout << "Enter starting rate of change" << endl;
  float rot;
  cin >> rot;
  while (distance > 0.1 || distance < -0.1)
    {
      rot += test(distance,rot);
      distance += rot;
      cout << "Distance: " << distance << endl << "Rate of change: " << rot << endl;
    }
}
void openTest()
{
  cout << "Enter distance" << endl;
  float distance;
  cin >> distance;
  cout << "Enter current rate of change" << endl;
  float rot;
  cin >> rot;
  test(distance,rot);
}
int main(int argv, char** argc)
{

  bool running = true;
  init();
  if (success)
    while (running)
      {
	cout << "Open or closed loop test? (o/c/q)" << endl;
	char selection;
	cin >> selection;
	if (selection == 'o')
	  openTest();
	else if (selection == 'c')
	  closedTest();
	else if (selection == 'q')
	  running = false;
      }
  else
    {
      printf("Parsing error, not continuing");
    }
}