#ifndef FUZZYRULE_H
#define FUZZYRULE_H

#include "fuzzyMembership.h"
#include "fuzzySet.h"
#include <vector>
#include <math.h>
using namespace std;

class fuzzyRule
{	
public:
	enum type
	{
		e_AND = 0,
		e_OR,
		e_NOT
	};

    fuzzyRule(char* _name, char* _input1, char* _input2, char* _output, type _type)
	{
		m_name = _name;
		m_input1 = _input1;
		m_input2 = _input2;
		m_output = _output;
		m_type = _type;
	}

	void evaluateSets(vector<fuzzySet>* _sets)
	{
		fuzzyMembership* param1 = NULL;
		fuzzyMembership* param2 = NULL;
		fuzzyMembership* param3 = NULL;

		for(int i = 0; i < 3; i++)
		{
			if(param1 == NULL)
			{ 
				for(unsigned int ii = 0; ii < _sets->size(); ii++)
				{
					param1 = (*_sets)[ii].find(m_input1); if(param1 != NULL) break;
				}
				if(param1 == NULL) return;
			}
			else if(param2 == NULL)
			{ 
				for(unsigned int ii = 0; ii < _sets->size(); ii++)
				{
					param2 = (*_sets)[ii].find(m_input2); if(param2 != NULL) break;
				}
				if(param2 == NULL) return;
			}
			else if(param3 == NULL)
			{
				for(unsigned int ii = 0; ii < _sets->size(); ii++)
				{
					param3 = (*_sets)[ii].find(m_output); if(param3 != NULL) break;
				}
				if(param3 == NULL) return;
			}
			else return; //invalid params
		}
		if(m_type == e_AND)
			param3->m_value = AND(param1->m_value, param2->m_value);
		else if (m_type == e_OR)
			param3->m_value = OR(param1->m_value, param2->m_value);
		else
			param3->m_value = NOT(param1->m_value);
	}

	static float AND(float _p1, float _p2){return min(_p1, _p2);}
	static float OR(float _p1, float _p2){return max(_p1, _p2);}
	static float NOT(float _p){return 1 - _p;}

	char* m_name;
	char* m_input1;
	char* m_input2;
	char* m_output;
	type m_type;


};


#endif//FUZZYRULE_H