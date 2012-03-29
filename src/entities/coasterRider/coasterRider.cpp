#include "coasterRider.h"
#include <cmath>

bool coasterRider::interpolateToPoint(vec3f _nextPoint)
{
	if(_nextPoint != m_currentPoint)
	{
		//update working points
		m_lastPoint = m_currentPoint;
		m_currentPoint = _nextPoint;
		//calc direction of movement
		m_dirVec = vec3f(m_currentPoint - m_lastPoint);
		//get length and normalise
		m_currentLength = m_dirVec.Length();
		m_dirVec.NormaliseSelf(); 
	}
	else if(m_mu + m_vel >= m_currentLength)
	{
		m_mu = 0;
		return true;
	}
	switch(m_mode)
	{
	case e_linearPhysics:
		{
			//PHYSICS!
			float gravity = 0.4f;
			m_greatestHeight = std::max(m_greatestHeight, m_pos.i);
			float KE = gravity*(m_greatestHeight - m_pos.i); //assume unit mass
			m_vel = std::sqrt(KE*0.5f);
			m_mu += std::max(m_vel,MIN_RIDER_VELOCITY);
			//setPos(point3f(m_dirVec.i * m_mu, m_dirVec.j * m_mu, m_dirVec.k * m_mu) + m_lastPoint);
			setPos((m_dirVec*m_mu)+m_lastPoint);
			break;
		}
	case e_linear:
		{
			m_vel = 0.5f;
			m_mu += m_vel;
			setPos((m_dirVec*m_mu)+m_lastPoint);
			break;
		}
	case e_nonLinear:
		//DO STUFF
		//printf("pos:(%f,%f,%f)\n",m_dirVec.i*m_mu,m_dirVec.j*m_mu,m_dirVec.k*m_mu);
		//point3f haha = point3f(m_lastPoint.m_x+m_dirVec.i*m_mu,m_lastPoint.m_y+m_dirVec.j*m_mu,m_lastPoint.m_z+m_dirVec.k*m_mu);
		//setPos(point3f(haha));
		break;
	}
	return false;
}
