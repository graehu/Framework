#ifndef vec3f_H
#define vec3f_H

#include <math.h>
#include <ostream>
//#include "point3f.h"
class vec2f;
using std::ostream;

class vec3f
{
public:
	// Constructors
	//vec3f(void);
	vec3f(float _i = 0, float _j = 0, float _k = 0);
	vec3f(const vec2f& rhs);
	//vec3f(point3f _point) : i(_point.m_x),j(_point.m_y),k(_point.m_z){}
	//vec3f(const vec3f & rhs);

	~vec3f(void);

	// Operations with other vectors
	vec3f & operator+=(const vec3f & rhs);
	vec3f & operator-=(const vec3f & rhs);

	// Special arithmetic
	float Dot3(const vec3f & rhs) const;
	vec3f Cross(const vec3f & rhs) const;

	vec3f & operator*=(const float s);
	vec3f & operator/=(const float s);

	bool operator==(const vec3f & rhs) const;
	bool operator!=(const vec3f& rhs) const {if(i!=rhs.i || j!=rhs.j || k!=rhs.k)return true;return false;}
	void operator=(const float & rhs);
	// Miscellaneous
	float Length() const;
	float LengthSqr() const;
	vec3f Normalise();
	void NormaliseSelf();
	void Dumpvec3f(char * s = 0);
	// Member data
	float i,j,k;
};
inline vec3f operator + (const vec3f &v1,
						   const vec3f &v2)
{
	return vec3f(v1.i + v2.i, v1.j + v2.j, v1.k + v2.k);
}
/*inline ostream& operator << (ostream& stream, vec3f& rhs)
{
    stream << " i: " << rhs.i << " j: " << rhs.j << " k: " << rhs.k;
    return stream;
}*/
inline vec3f operator - (const vec3f &v1,
						   const vec3f &v2)
{
	return vec3f(v1.i - v2.i, v1.j - v2.j, v1.k - v2.k);
}

inline vec3f operator - (const vec3f &v1)
{
	return vec3f(-v1.i, -v1.j, -v1.k);
}

inline vec3f operator * (const vec3f &v,
						   const float &s)
{
	return vec3f(v.i * s, v.j * s, v.k * s);
}

inline vec3f operator * (const float & s,
						   const vec3f &v)
{
	return vec3f(v.i * s, v.j * s, v.k * s);
}

inline vec3f operator / (const vec3f &v,
						   const float & s)
{
	return vec3f(v.i / s, v.j / s, v.k / s);
}

inline vec3f CrossProduct (const vec3f &v1,
							 const vec3f &v2)
{
	return vec3f((v1.j * v2.k) - (v1.k * v2.j),
				   (v1.k * v2.i) - (v1.i * v2.k),
				   (v1.i * v2.j) - (v1.j * v2.i));
}


inline float DotProduct3(const vec3f &v1,
						 const vec3f &v2)
{
	return (v1.i * v2.i + v1.j * v2.j + v1.k * v2.k);
}

inline vec3f Normalise (const vec3f &v)
{
	return v / v.Length();
}

#endif // vec3f_H
