#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "../entities/entity.h"

class controller
{
public:
	controller(entity* _entity)
	{
		m_entity = _entity;
	}
	~controller(){};
	virtual void update(float _dt) = 0;
protected:

	entity* m_entity;

private:

};

#endif//CONTROLLER_H