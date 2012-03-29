#include "fuzzyController.h"

fuzzyController::fuzzyController(entity* _entity, input* _input)
	:controller(_entity)
{
	m_input = _input;
	fuzzySet distanceSet("distance");
	fuzzySet speedSet("speed");
	fuzzySet steerSet("steer");

	char* distances[] = {"LargeDistL", "SmallDistL", "NoDist", "SmallDistR", "LargeDistR"};
	char* speeds[] = {"FastSpeedL", "SlowSpeedL", "NoSpeed", "SlowSpeedR", "FastSpeedR"};
	char* steerDegrees[] = {"HardTurnL", "SoftTurnL", "NoTurn", "SoftTurnR", "HardTurnR"};

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
	m_sets.push_back(distanceSet);
	m_sets.push_back(speedSet);
	m_sets.push_back(steerSet);
	///rules.
	m_rules.push_back(fuzzyRule("HardLeft", distances[4], speeds[4], steerDegrees[0], fuzzyRule::e_OR));
	m_rules.push_back(fuzzyRule("SoftLeft", distances[3], speeds[3], steerDegrees[1], fuzzyRule::e_OR));
	m_rules.push_back(fuzzyRule("Straight", distances[2], speeds[2], steerDegrees[2], fuzzyRule::e_OR));
	m_rules.push_back(fuzzyRule("HardRight", distances[1], speeds[1], steerDegrees[3], fuzzyRule::e_OR));
	m_rules.push_back(fuzzyRule("SoftRight", distances[0], speeds[0], steerDegrees[4], fuzzyRule::e_OR));

	m_linePos = 100;
	m_entity->setXPos(m_linePos);
	m_angle = 0;
	m_distance = 0;
}