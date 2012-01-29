#ifndef QUATERNION_H 
#define QUATERNION_H 
#include <math.h>
#include "mat4x4f.h"

#define PI			3.14159265358979323846

class quaternion  
{
public:
	quaternion operator *(quaternion q);
	void createMatrix(Mat4x4f *pMatrix);
	void createFromAxisAngle(float x, float y, float z, float degrees);
	quaternion();
	virtual ~quaternion();

private:

	float m_w;
	float m_i;
	float m_j;
	float m_k;
};

#endif//QUATERNION_H 
