#ifndef CAR_H
#define CAR_H

#include "../../ai/fuzzyLogic/fuzzyController.h"
#include "../entity.h"
#include "../../graphics/renderable/sprite/sprite.h"

class car : public entity
{
public:
	car(input* _input)
	{
		m_controller = new fuzzyController(this, _input);
		m_type = e_car;
		m_sprite = new sprite("data/car.png");
	}
	~car(){}


	void update(float _dt)
	{
		
		m_controller->update(_dt);


		m_sprite->m_x = getXPos();
		m_sprite->m_y = getYPos();
	}
	void render(iRenderVisitor* renderer){m_sprite->render(renderer);}


private:
protected:
	sprite* m_sprite;
	controller* m_controller;
};
#endif//CAR_H