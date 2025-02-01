#ifndef CAMERA_H
#define CAMERA_H

#include "../../types/quaternion.h"
#include "../../types/vec3f.h"
#include "../../types/mat4x4f.h"
#include "../renderable/iRenderable.h"

class camera  : public iRenderable
{
public:

   void changeForwardVelocity(float vel);
   void changeStrafeVelocity(float vel);
   void changeHeading(float degrees);
   void changePitch(float degrees);

   //void setPrespective(void);
   void render(iRenderVisitor* _renderer){_renderer->visit(this);}
   mat4x4f getView(void){return m_view;}
   // todo: why the hell am I returning so many negatives below :(
   vec3f getPosition(void){return vec3f(m_position.i, m_position.j, m_position.k);}
   vec3f getDirection(void){return vec3f(-m_DirectionVector.i, -m_DirectionVector.j, m_DirectionVector.k);}
   vec3f getStrafeDirection(void){return vec3f(-m_strafeVector.i, -m_strafeVector.j, m_strafeVector.k);}

   void setPosition(vec3f _position){m_position = _position;}
   void setViewMatrix(mat4x4f _matrix){m_view = _matrix;}
   vec3f getUp(void){return m_up;}

   void update(void);
   camera();
	
   float m_maxPitchRate;
   float m_maxHeadingRate;
   float m_headingDegrees;
   float m_pitchDegrees;
   float m_maxVelocity;
   float m_forwardVelocity;
   float m_strafeVelocity;

   quaternion m_qHeading;
   quaternion m_qPitch;

   mat4x4f m_view;
	
   vec3f m_up;
   vec3f m_position;
   vec3f m_DirectionVector;
   vec3f m_strafeVector;

};

#endif//CAMERA_H
