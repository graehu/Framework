#ifndef ENTITY_H
#define ENTITY_H
#include "../types/vec3f.h"

// In a real game this list would get considerably bigger.
enum entityType
{
    e_entity = 0,
    e_netEntity,
    e_player,
    e_bot,
	e_coasterRider,
	e_car,
    e_totalEntities
};

class entity
{
public:

	entity() : m_pos(), m_type(e_entity){}
	~entity(){}
	
    float getXPos(void){return m_pos.i;}
    float getYPos(void){return m_pos.j;}
    float getZPos(void){return m_pos.k;}
    void setXPos(float _x){m_pos.i = _x;}
    void setYPos(float _y){m_pos.j = _y;}
    void setZPos(float _z){m_pos.k = _z;}
	void setPos(vec3f _pos){m_pos = _pos;}
	vec3f getPos(void){return m_pos;}

	virtual void update(float _dt) = 0;

	entityType getType(void){return m_type;}

protected:

    vec3f m_pos;
	entityType m_type;

private:
};

#endif//ENTITY_H
