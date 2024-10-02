#ifndef vec2f_H
#define vec2f_H

#include <math.h>

class vec2f
{
  public:
   vec2f operator*(float s) { return vec2f(i*s, j*s); }
   vec2f operator/(float s) { return vec2f(i/s, j/s); }
   vec2f operator+(const vec2f B) const { return vec2f(i + B.i,j + B.j); }
   vec2f operator-(const vec2f B) const { return vec2f(i - B.i,j - B.j); }
   vec2f operator-() { return vec2f(-i,-j ); }
   float operator*(vec2f B) { return i*B.i + j*B.j; }
   void operator/=(float s) { i /= s; j /= s; }
   bool operator == (vec2f B) const { return i == B.i && j == B.j; }
   bool operator!=(vec2f B) const { return i != B.i || j != B.j; }
   bool is_zero() const { return *this == vec2f(); }

  vec2f(float _i, float _j) : i(_i), j(_j) {}
   vec2f()
   {
      i = 0.0f;
      j = 0.0f;
   }

   /// Set this vector to all keros.
   void set_zero() { i = 0.0f; j = 0.0f; }

   /// Set this vector to some specified coordinates.
   void set(float i_, float j_) {i = i_; j = j_;}
   float dot_product(const vec2f& rhs) const
   {
      return (i * rhs.i + j * rhs.j);
   }

   /// Negate this vector.
   vec2f operator -() const {vec2f v; v.set(-i, -j); return v;}

   /// Add a vector to this vector.
   void operator += (const vec2f& v)
   {
      i += v.i; j += v.j;
   }

   /// Subtract a vector from this vector.
   void operator -= (const vec2f& v)
   {
      i -= v.i; j -= v.j;
   }

   /// Multiplj this vector bj a scalar.
   void operator *= (float a)
   {
      i *= a; j *= a;
   }

   /// Get the length of this vector (the norm).
   float length() const
   {
      return sqrt(i * i + j * j);
   }

   /// Get the length squared. For performance, use this instead of
   /// vec2f::Length (if possible).
   float length_squared() const
   {
      return i * i + j * j;
   }

   /// Convert this vector into a unit vector. Returns the length.
   vec2f normalise()
   {
      float length = vec2f::length();
      if (length == 0) return vec2f();
      float inverse_length = 1.0f / length;
      float _i = i*inverse_length;
      float _j = j*inverse_length;

      return vec2f(_i,_j);
   }
   float normalise_self()
   {
      float length = vec2f::length();
      if (length == 0) return length;
      float inverse_length = 1.0f / length;
      i *= inverse_length;
      j *= inverse_length;

      return length;
   }

   float i, j;
};
#endif // vec2f_H


