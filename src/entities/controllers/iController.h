#ifndef ICONTROLLER_H
#define ICONTROLLER_H

#include "../entity.h"

//Have commands here?

class iController
{
public:
	iController(entity _entity)
	{
		m_entity = _entity;
	}
	void update(float _deltaTime) = 0;

protected:
private:
	entity* m_entity;
};

#endif//ICONTROLLER_H
