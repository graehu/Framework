#ifndef FUZZYSET_H
#define FUZZYSET_H

#include "fuzzyMembership.h"
#include <vector>

class fuzzySet
{
public:

	fuzzySet(char* _name):m_name(_name){}
	enum type
	{
		e_centroid = 0,
		e_mean
	};
	float defuzzify(type _type)
	{
		if(m_memberships.empty()) return 0;

		switch(_type)
		{
			case e_centroid:
				break;
			case e_mean: //find mean of two highest memberships
				{
					m_biggest = &m_memberships[0];
					m_secBiggest = m_biggest;
					for(unsigned int i = 0; i < m_memberships.size(); i++)
					{
						if(m_memberships[i].m_value > m_biggest->m_value) 
						{
							m_secBiggest = m_biggest;
							m_biggest = &m_memberships[i];
						}
						else if(m_memberships[i].m_value > m_secBiggest->m_value) m_secBiggest = &m_memberships[i];
					}
					if(m_biggest->m_value > 0 && m_secBiggest->m_value > 0)
						return ((m_biggest->m_value*m_biggest->m_midRange)+(m_secBiggest->m_value*m_secBiggest->m_midRange))/2;//Weeeeeeeeee~ 
					else if(m_biggest->m_value > 0)
						return m_biggest->m_midRange*m_biggest->m_value;
					else
						return 0;
				}
		}
		return 0;
	}
	void update(float _crisp)
	{
		for(unsigned int i = 0; i < m_memberships.size(); i++)
			m_memberships[i].update(_crisp);
	}

	fuzzyMembership* find(char* _input)
	{
		for(unsigned int i = 0; i < m_memberships.size(); i++)
			if(_input == m_memberships[i].m_name)
				return &m_memberships[i];

		return NULL;
	}
	vector<fuzzyMembership> m_memberships;
	fuzzyMembership* m_biggest;
	fuzzyMembership* m_secBiggest;

	char* m_name;
};
#endif//FUZZYSET_H
