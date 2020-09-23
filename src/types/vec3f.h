#ifndef vec3f_H
#define vec3f_H

#include <math.h>
class vec2f;

class vec3f
{
public:
   // Constructors
   vec3f(float _i = 0, float _j = 0, float _k = 0);
   vec3f(const vec2f& rhs);

   ~vec3f(void);

   // Operations with other vectors
   vec3f& operator+=(const vec3f& rhs);
   vec3f& operator-=(const vec3f& rhs);

   // Special arithmetic
   float dot_product3(const vec3f& rhs) const;
   float dot_product2(const vec3f& rhs) const;
   vec3f cross_product(const vec3f& rhs) const;
   vec3f limit(float _length) const;

   vec3f & operator*=(const float s);
   vec3f & operator/=(const float s);

   bool operator==(const vec3f & rhs) const;
   bool operator!=(const vec3f& rhs) const {if(i!=rhs.i || j!=rhs.j || k!=rhs.k)return true;return false;}
   void operator=(const float& rhs);
   // Miscellaneous
   float length() const;
   float length_squared() const;
   vec3f normalise();
   void normalise_self();
   // Member data
   float i,j,k;
};
inline vec3f operator + (const vec3f &v1, const vec3f &v2)
{
   return vec3f(v1.i + v2.i, v1.j + v2.j, v1.k + v2.k);
}
inline vec3f operator - (const vec3f &v1, const vec3f &v2)
{
   return vec3f(v1.i - v2.i, v1.j - v2.j, v1.k - v2.k);
}

inline vec3f operator - (const vec3f &v1)
{
   return vec3f(-v1.i, -v1.j, -v1.k);
}

inline vec3f operator * (const vec3f &v, const float &s)
{
   return vec3f(v.i * s, v.j * s, v.k * s);
}

inline vec3f operator * (const float & s, const vec3f &v)
{
   return vec3f(v.i * s, v.j * s, v.k * s);
}

inline vec3f operator / (const vec3f &v, const float & s)
{
   return vec3f(v.i / s, v.j / s, v.k / s);
}

inline vec3f cross_product (const vec3f &v1, const vec3f &v2)
{
   return vec3f((v1.j * v2.k) - (v1.k * v2.j),
		(v1.k * v2.i) - (v1.i * v2.k),
		(v1.i * v2.j) - (v1.j * v2.i));
}
inline float dot_product(const vec3f &v1, const vec3f &v2)
{
   return (v1.i * v2.i + v1.j * v2.j + v1.k * v2.k);
}
inline vec3f normalise (const vec3f &v)
{
   return v / v.length();
}

#endif // vec3f_H
