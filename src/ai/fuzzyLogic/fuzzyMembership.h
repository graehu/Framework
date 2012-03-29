#pragma once
#include "../../types/vec3f.h"
#include <string>
using namespace std;


class fuzzyMembership
{
public:

	enum type
	{
		e_base = 0,
		e_triangle,
		e_plateau	
	};

	fuzzyMembership(char* _name, vec3f _range, type _type)
		:m_name(_name), m_range(_range), m_type(_type)
	{
		m_midRange = (_range.i + _range.j) * 0.5f;
		m_height = 1;
		m_value = 0;
	}

	~fuzzyMembership(){}
	
	void update(float _crisp)
	{
		m_midRange = (m_range.i + m_range.j) * 0.5f;
		if(_crisp < m_range.i || _crisp > m_range.j)
		{
			m_value = 0;
			return;
		}
		switch(m_type)
		{
		case e_triangle:
			{
				float theta = 0;
				vec3f temp1(1,0);
				vec3f temp2((m_range.j-m_range.i)*0.5f, m_height);
				temp2.NormaliseSelf();
				
				//find smallest distance to min or max range
				float relValue = min(abs(m_range.i - _crisp), abs(m_range.j - _crisp));
				theta = acos(DotProduct3(temp1, temp2));
				m_value = abs(relValue * tan(theta));
			}
			break;
		case e_plateau:///he doesn't even do anything with his freaking trapazoids~
			break;
			
		}
	}

	char* m_name;
	vec3f m_range;
	float m_midRange, m_height, m_value;
	type m_type;
};
