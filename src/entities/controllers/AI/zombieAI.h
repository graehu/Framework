#ifndef ZOMBIEAI_H
#define ZOMBIEAI_H

#include "../iController.h"

class zombieAI : public iController
{
public:
	zombieAI(entity _entity) : iController(_entity){}
	void update(float _deltaTime){}
};


#endif//ZOMBIEAI_H
