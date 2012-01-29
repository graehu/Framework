#ifndef ENTITY_H
#define ENTITY_H
#include "../types/point3f.h"

// In a real game this list would get considerably bigger.
enum entityType
{
    e_entity = 0,
    e_netEntity,
    e_player,
    e_bot,
    e_totalEntities
};

class entity
{
public:

	entity() : m_pos(), m_type(e_entity){}
	~entity(){}

    float getXPos(void){return m_pos.m_x;}
    float getYPos(void){return m_pos.m_y;}
    float getZPos(void){return m_pos.m_z;}
    void setXPos(float _x){m_pos.m_x = _x;}
    void setYPos(float _y){m_pos.m_y = _y;}
    void setZPos(float _z){m_pos.m_z = _z;}

	entityType getType(void){return m_type;}

protected:

    point3f m_pos;
	entityType m_type;

private:
};

#endif//ENTITY_H
