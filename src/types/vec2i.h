#ifndef VEC2I_H
#define VEC2I_H
#include <math.h>
#include "vec2f.h"

struct vec2i
{
	int i;
	int j;

	vec2i( int pi = 0, int pj = 0 ) : i( pi ), j( pj ) { }
	~vec2i(){}

	vec2i operator*( int s ) { return vec2i( i*s, j*s ); }
	vec2i operator/( int s ) { return vec2i( i/s, j/s ); }
	vec2i operator+( int s ) { return vec2i( i+s, j+s ); }
	vec2i operator+(const vec2i B ) const{ return vec2i( i + B.i, j + B.j ); }
	vec2i operator-(const vec2i B ) const{ return vec2i( i - B.i, j - B.j ); }
	vec2i operator-() { return vec2i( -i, -j ); }
	int operator*( vec2i B ) { return i*B.i + j*B.j; }
	void operator=( vec2i B ) { i = B.i; j = B.j; }
	void operator+=( vec2i B ) { i += B.i; j += B.j; }
	void operator-=( vec2i B ) { i -= B.i; j -= B.j; }
	void operator*=( int s ) { i *= s; j *= s; }
	void operator/=( int s ) { i /= s; j /= s; }
	bool operator==( vec2i B ) { return i == B.i && j == B.j; }
	bool operator!=( vec2i B ) { return i != B.i || j != B.j; }
	int length() { return sqrt( float(i*i) + float(j*j) ); }
};

#endif // VEC2I_H
