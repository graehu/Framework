#ifndef FUZZYCONTROLLER_H
#define FUZZYCONTROLLER_H

#include "fuzzySet.h"
#include "fuzzyRule.h"
#include "fuzzyMembership.h"
#include "../../input/input.h"
#include "../controller.h"

#define CARSPEED 1.0f
#define TURNSPEED 1.0f

class fuzzyController : public controller
{
public:
	fuzzyController(entity* _entity, input* _input);
	~fuzzyController(){}
	void update(float _dt)
	{
		float distanceToLine = m_linePos - (m_entity->getXPos()/3.2f);
		float speedToLine = (distanceToLine - m_distance) * _dt * CARSPEED;///not sure what 45  is for
		//update input sets.
		if(distanceToLine > 1.2f && distanceToLine > 0)distanceToLine = 1.2f;
		else if(distanceToLine < 0 && distanceToLine < -1.2f)distanceToLine = -1.2f;
		if(speedToLine > 1.2f && speedToLine > 0)speedToLine = 1.2f;
		else if(speedToLine < 0 && speedToLine < -1.2f)speedToLine = -1.2f;
		
		m_distance = distanceToLine;

		m_sets[0].update(-distanceToLine);
		m_sets[1].update(-speedToLine);
		//evaluate rules.
		for(unsigned int i = 0; i < m_rules.size(); i++)
			m_rules[i].evaluateSets(&m_sets);
		//m_sets[2]

		//if not being moved by the player.
		if(m_input->isKeyPressed(input::e_left))
		{
			m_angle -= TURNSPEED * _dt;
			if(m_angle < -1.2f) m_angle = -1.2f;
		}
		else if(m_input->isKeyPressed(input::e_right))
		{
			m_angle += TURNSPEED * _dt;
			if(m_angle > 1.2f) m_angle = 1.2f;
		}
		else
		{
			float newAngle = m_sets[2].defuzzify(fuzzySet::e_mean);
			float deltaAngle = newAngle - m_angle;
			m_angle += deltaAngle * TURNSPEED * _dt;
			printf("steering type: %s ", m_sets[2].m_biggest->m_name);
		}
		printf("angle: %f distance: %f\n", m_angle, m_distance);
		m_entity->setXPos(m_entity->getXPos()+((m_angle*CARSPEED)*3.2f));
	}
	void setLinePosition(float _x){m_linePos = _x;}
	float getLinePosition(void){return m_linePos;}


protected:

private:

	//
	float m_distance;
	float m_angle;
	float m_linePos; //all values in x
	input* m_input;
	//

	std::vector<fuzzySet> m_sets;
	std::vector<fuzzyRule> m_rules;
};

#endif//FUZZYCONTROLLER_H