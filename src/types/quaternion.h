#ifndef QUATERNION_H 
#define QUATERNION_H 
#include <math.h>
#include "mat4x4f.h"
#include <cassert>

#define PI			3.14159265358979323846
const float epsilon = 0.00001f;                         ///< floating point epsilon for single precision. todo: verify epsilon value and usage
const float epsilonSquared = epsilon * epsilon;         ///< epsilon value squared

class quaternion  
{
public:
	quaternion(float _w, float _i, float _j,float _k)
		:w(_w),i(_i),j(_j),k(_k){};
	quaternion(mat4x4f _matrix);
	quaternion(float _angle, vec3f _axis);
	quaternion();
	//quaternion();

	virtual ~quaternion();

	void createMatrix(mat4x4f *pMatrix);
	void createFromAxisAngle(float x, float y, float z, float degrees);
	void normalise(void);
	float length(void);
	void identity();
	//operators
	//quaternion operator *(quaternion q);
	quaternion operator *(quaternion q) const;
	quaternion operator *(float _s) const;
	quaternion operator +(quaternion q) const;
	void operator+=(quaternion q);
	float& operator [](int _i);

	float w;
	float i;
	float j;
	float k;

private:
};

inline quaternion slerp(const quaternion &a, const quaternion &b, float t)
{
	assert(t>=0);
	assert(t<=1);
			
	float flip = 1;

	float cosine = a.w*b.w + a.i*b.i + a.j*b.j + a.k*b.k;
	
	if (cosine<0) 
	{ 
		cosine = -cosine; 
		flip = -1; 
	} 
	
	if ((1-cosine)<epsilon) 
		return a * (1-t) + b * (t*flip); 
	
	float theta = (float)acos(cosine); 
	float sine = (float)sin(theta); 
	float beta = (float)sin((1-t)*theta) / sine; 
	float alpha = (float)sin(t*theta) / sine * flip; 
	
	return a * beta + b * alpha; 
}

	
#endif//QUATERNION_H 
