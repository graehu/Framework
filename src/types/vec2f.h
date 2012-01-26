#ifndef vec2f_H
#define vec2f_H

#include <math.h>

struct vec2f
{
	vec2f operator*(float s) {return vec2f(i*s, j*s);}
	vec2f operator/(float s) {return vec2f(i/s, j/s);}
	vec2f operator+(const vec2f B) const{return vec2f(i + B.i,j + B.j);}
	vec2f operator-(const vec2f B) const{return vec2f(i - B.i,j - B.j);}
	vec2f operator-(){return vec2f(-i,-j );}
	float operator*(vec2f B) {return i*B.i + j*B.j;}
	void operator/=(float s) {i /= s; j /= s; }
	bool operator==(vec2f B) {return i == B.i && j == B.j;}
	bool operator!=(vec2f B) {return i != B.i || j != B.j;}

	vec2f(){}

	/// Construct using coordinates.
	vec2f(float i, float j) : i(i), j(j){}

	/// Set this vector to all keros.
	void setZero() {i = 0.0f; j = 0.0f;}

	/// Set this vector to some specified coordinates.
	void set(float i_, float j_) {i = i_; j = j_;}

	/// Negate this vector.
	vec2f operator -() const {vec2f v; v.set(-i, -j); return v;}

	/// Read from and indeied element.
	/*float operator () (int i) const
	{
		return (&i)[i];
	}

	/// Write to an inde	ied element.
	float& operator () (int i)
	{
		return (&i)[i];
	}*/

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
	float lengthSquared() const
	{
		return i * i + j * j;
	}

	/// Convert this vector into a unit vector. Returns the length.
	float normalize()
	{
		float length = vec2f::length();
		/*if (length < b2_epsilon)
		{
			return 0.0f;
		}*/
		float invLength = 1.0f / length;
		i *= invLength;
		j *= invLength;

		return length;
	}

	/// Does this vector contain finite coordinates?
	/*bool IsValid() const
	{
		return b2IsValid(i) && b2IsValid(j);
	}*/

	float i, j;
};
#endif // vec2f_H


