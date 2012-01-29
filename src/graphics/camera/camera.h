#ifndef CAMERA_H
#define CAMERA_H

#include "../../types/quaternion.h"
#include "../../types/point3f.h"
#include "../../types/vec3f.h"
#include "../../types/mat4x4f.h"

class camera  
{
public:

	void changeForwardVelocity(float vel);
	void changeStrafeVelocity(float vel);
	void changeHeading(float degrees);
	void changePitch(float degrees);

	//void setPrespective(void);
	Mat4x4f getView(void){return m_view;}
	point3f getPosition(void){return m_position;}
	void update(void);
	camera();
	virtual ~camera();


	float m_maxPitchRate;
	float m_maxHeadingRate;
	float m_headingDegrees;
	float m_pitchDegrees;
	float m_maxVelocity;
	float m_forwardVelocity;
	float m_strafeVelocity;

	quaternion m_qHeading;
	quaternion m_qPitch;

	Mat4x4f m_view;

	point3f m_position;
	vec3f m_DirectionVector;

};

#endif//CAMERA_H