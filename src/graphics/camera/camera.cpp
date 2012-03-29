#include "camera.h"
#include <math.h>

template <typename T> int sgn(T val)
{
return (val > T(0)) - (val < T(0));
}


//TODO: make this camera more user friendly. probably need to add deceleration

camera::camera()
{
	// Initalize all our member varibles.


	m_maxVelocity			= 2.5f;
	m_maxPitchRate			= 2.5f;
	m_maxHeadingRate		= 2.5f;

	m_headingDegrees		= 0.0f;
	m_pitchDegrees			= 0.0f;
	m_forwardVelocity		= 0.0f;
	m_strafeVelocity		= 0.0f;



}

camera::~camera()
{

}

void camera::update()
{
	Mat4x4f Matrix;
	quaternion q;

	// Make the Quaternions that will represent our rotations
	m_qPitch.createFromAxisAngle(1.0f, 0.0f, 0.0f, m_pitchDegrees);
	m_qHeading.createFromAxisAngle(0.0f, 1.0f, 0.0f, m_headingDegrees);
	
	// Combine the pitch and heading rotations and store the results in q
	q = m_qPitch * m_qHeading;
	q.createMatrix(&Matrix);
	
	m_view = Matrix;
	
	// Create a matrix from the pitch Quaternion and get the j vector 
	// for our direction.

	//transposed all maths from opengl

	m_qPitch.createMatrix(&Matrix);
	m_DirectionVector.j = Matrix.elem[2][1];//[9];

	// Combine the heading and pitch rotations and make a matrix to get
	// the i and k vectors for our direction.

	q = m_qHeading * m_qPitch;
	q.createMatrix(&Matrix);
	m_DirectionVector.i = Matrix.elem[2][0];//[8]
	m_DirectionVector.k = Matrix.elem[2][2];//[10];

	m_DirectionVector.NormaliseSelf();
	// Scale the direction by our speed.
	vec3f up(0,1,0); 
	vec3f strafeDirection;
	strafeDirection = CrossProduct(m_DirectionVector, up);
	//TODO: Find out why normalising here isn't going well.
	strafeDirection.NormaliseSelf();
	m_strafeVector = strafeDirection;
	m_up.i = m_view.elem[0][1];
	m_up.j = m_view.elem[1][1];
	m_up.k = m_view.elem[2][1];

	// Increment our position by the vector
	m_position.i += (m_DirectionVector.i*m_forwardVelocity) + (strafeDirection.i*m_strafeVelocity);
	m_position.j += (m_DirectionVector.j*m_forwardVelocity) + (strafeDirection.j*m_strafeVelocity);
	m_position.k += (m_DirectionVector.k*m_forwardVelocity) + (strafeDirection.k*m_strafeVelocity);

	Mat4x4f temp; //= Matrix;
	temp.elem[0][3] = -m_position.i;
	temp.elem[1][3] = -m_position.j;
	temp.elem[2][3] = m_position.k;

	temp = Transpose(temp);
	m_view = temp*m_view;

	if(m_strafeVelocity > 0) m_strafeVelocity -= 0.0005f;
	else if(m_strafeVelocity < 0) m_strafeVelocity += 0.0005f;
	
	if(m_strafeVelocity < 0.0005 && m_strafeVelocity > -0.0005)
		m_strafeVelocity = 0;

	if(m_forwardVelocity > 0)  m_forwardVelocity -= 0.0005f;
	else if(m_forwardVelocity < 0) m_forwardVelocity += 0.0005f;

	if(m_forwardVelocity < 0.0005 && m_forwardVelocity > -0.0005)
		m_forwardVelocity = 0;

}

void camera::changePitch(float degrees)
{
	if(fabs(degrees) < fabs(m_maxPitchRate))
	{
		// Our pitch is less than the max pitch rate that we 
		// defined so lets increment it.
		m_pitchDegrees += degrees;
	}
	else
	{
		// Our pitch is greater than the max pitch rate that
		// we defined so we can only increment our pitch by the 
		// maximum allowed value.
		if(degrees < 0)
		{
			// We are pitching down so decrement
			m_pitchDegrees -= m_maxPitchRate;
		}
		else
		{
			// We are pitching up so increment
			m_pitchDegrees += m_maxPitchRate;
		}
	}

	// We don't want our pitch to run away from us. Although it
	// really doesn't matter I prefer to have my pitch degrees
	// within the range of -360.0f to 360.0f
	if(m_pitchDegrees > 360.0f)
	{
		m_pitchDegrees -= 360.0f;
	}
	else if(m_pitchDegrees < -360.0f)
	{
		m_pitchDegrees += 360.0f;
	}
}

void camera::changeHeading(float degrees)
{
	if(fabs(degrees) < fabs(m_maxHeadingRate))
	{
		// Our Heading is less than the max heading rate that we 
		// defined so lets increment it but first we must check
		// to see if we are inverted so that our heading will not
		// become inverted.
		if(m_pitchDegrees > 90 && m_pitchDegrees < 270 || (m_pitchDegrees < -90 && m_pitchDegrees > -270))
		{
			m_headingDegrees -= degrees;
		}
		else
		{
			m_headingDegrees += degrees;
		}
	}
	else
	{
		// Our heading is greater than the max heading rate that
		// we defined so we can only increment our heading by the 
		// maximum allowed value.
		if(degrees < 0)
		{
			// Check to see if we are upside down.
			if((m_pitchDegrees > 90 && m_pitchDegrees < 270) || (m_pitchDegrees < -90 && m_pitchDegrees > -270))
			{
				// Ok we would normally decrement here but since we are upside
				// down then we need to increment our heading
				m_headingDegrees += m_maxHeadingRate;
			}
			else
			{
				// We are not upside down so decrement as usual
				m_headingDegrees -= m_maxHeadingRate;
			}
		}
		else
		{
			// Check to see if we are upside down.
			if(m_pitchDegrees > 90 && m_pitchDegrees < 270 || (m_pitchDegrees < -90 && m_pitchDegrees > -270))
			{
				// Ok we would normally increment here but since we are upside
				// down then we need to decrement our heading.
				m_headingDegrees -= m_maxHeadingRate;
			}
			else
			{
				// We are not upside down so increment as usual.
				m_headingDegrees += m_maxHeadingRate;
			}
		}
	}
	
	// We don't want our heading to run away from us either. Although it
	// really doesn't matter I prefer to have my heading degrees
	// within the range of -360.0f to 360.0f
	if(m_headingDegrees > 360.0f)
	{
		m_headingDegrees -= 360.0f;
	}
	else if(m_headingDegrees < -360.0f)
	{
		m_headingDegrees += 360.0f;
	}
}

void camera::changeForwardVelocity(float vel)
{
	if(fabs(vel) < fabs(m_maxVelocity))
	{
		// Our velocity is less than the max velocity increment that we 
		// defined so lets increment it.
		m_forwardVelocity += vel;
	}
	else
	{
		// Our velocity is greater than the max velocity increment that
		// we defined so we can only increment our velocity by the 
		// maximum allowed value.
		if(vel < 0)
		{
			// We are slowing down so decrement
			m_forwardVelocity -= -m_maxVelocity;
		}
		else
		{
			// We are speeding up so increment
			m_forwardVelocity += m_maxVelocity;
		}
	}
}

void camera::changeStrafeVelocity(float vel)
{
	if(fabs(vel) < fabs(m_maxVelocity))
	{
		// Our velocity is less than the max velocity increment that we 
		// defined so lets increment it.
		m_strafeVelocity += vel;
	}
	else
	{
		// Our velocity is greater than the max velocity increment that
		// we defined so we can only increment our velocity by the 
		// maximum allowed value.
		if(vel < 0)
		{
			// We are slowing down so decrement
			m_strafeVelocity -= -m_maxVelocity;
		}
		else
		{
			// We are speeding up so increment
			m_strafeVelocity += m_maxVelocity;
		}
	}
}