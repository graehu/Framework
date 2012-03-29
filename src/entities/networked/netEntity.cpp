#include "netEntity.h"
#include "../../networking/utils/dataUtils.h"

using namespace net;

netEntity::netEntity()
{
    m_type = e_netEntity;
    m_command = ' ';
}

void netEntity::readPacket(packet* _packet)
{
	m_pos.i = (float)_packet->iterRead<int>();
	m_pos.j = (float)_packet->iterRead<int>();
}

void netEntity::writePacket(packet* _packet)
{
	_packet->iterWrite((unsigned int)m_pos.i);
	_packet->iterWrite((unsigned int)m_pos.j);
}

void netEntity::move(void)
{
    if((m_command&1) > 0) m_pos.j-=1;
    if((m_command&2) > 0) m_pos.i-=1;
    if((m_command&4) > 0) m_pos.j+=1;
    if((m_command&8) > 0) m_pos.i+=1;
}

