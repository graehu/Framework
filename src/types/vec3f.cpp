#include <stdio.h>
#include "vec3f.h"
#include "vec2f.h"
vec3f::vec3f(const vec2f& rhs)
{
   *this = vec3f(rhs.i,rhs.j,0);
}
vec3f::vec3f(float _i, float _j, float _k)	
   :i(_i),j(_j),k(_k)
{
}

vec3f& vec3f::operator +=(const vec3f& rhs)
{
   i += rhs.i;
   j += rhs.j;
   k += rhs.k;

   return *this;
}

vec3f& vec3f::operator -=(const vec3f& rhs)
{
   i -= rhs.i;
   j -= rhs.j;
   k -= rhs.k;

   return *this;
}

float vec3f::dot_product3(const vec3f& rhs) const
{
   return (i * rhs.i + j * rhs.j + k * rhs.k);
}
float vec3f::dot_product2(const vec3f& rhs) const
{
   return (i * rhs.i + j * rhs.j);
}
vec3f vec3f::cross_product(const vec3f& rhs) const
{
   return vec3f(j * rhs.k - k * rhs.j, k * rhs.i - i * rhs.k, i * rhs.j - j * rhs.i);
}
vec3f vec3f::limit(float _length) const
{
   float len = length();
   if(len > _length)
   {
      vec3f temp = vec3f(i,j,k);
      temp /= len;
      return temp *_length;
   }
   return vec3f(i,j,k);
}
vec3f& vec3f::operator *=(const float s)
{
   i *= s;
   j *= s;
   k *= s;

   return *this;
}

vec3f& vec3f::operator /=(const float s)
{
   if(s != 0)
   {
      i /= s;
      j /= s;
      k /= s;
   }
   return *this;
}

float vec3f::length() const
{
   return sqrt((i * i) + (j * j) + (k * k));
}

float vec3f::length_squared() const
{
   return (i * i + j * j + k * k);
}
bool vec3f::operator ==(const vec3f& rhs) const
{
   return ((i == rhs.i) && (j == rhs.j) && (k == rhs.k));
}
void vec3f::operator=(const float& rhs)
{
   i = rhs;
   j = rhs;
   k = rhs;
}
vec3f vec3f::normalise()
{
   float len = this->length();
   if (len != 0) return (*this / this->length());
   else return vec3f();
}

void vec3f::normalise_self()
{
   float len = this->length();
   if (len != 0) *this /= len;
   else *this = vec3f();
}
