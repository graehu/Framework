#ifndef VEC2I_H
#define VEC2I_H
#include <math.h>
#include "vec2f.h"

struct Vec2i { //Simple vector class
	int i;
	int j;

	Vec2i( int pi = 0, int pj = 0 ) : i( pi ), j( pj ) { }
	~Vec2i(){}

	Vec2i operator*( int s ) { return Vec2i( i*s, j*s ); }
	Vec2i operator/( int s ) { return Vec2i( i/s, j/s ); }
	Vec2i operator+( int s ) { return Vec2i( i+s, j+s ); }
	Vec2i operator+(const Vec2i B ) const{ return Vec2i( i + B.i, j + B.j ); }
	Vec2i operator-(const Vec2i B ) const{ return Vec2i( i - B.i, j - B.j ); }
	Vec2i operator-() { return Vec2i( -i, -j ); }
	int operator*( Vec2i B ) { return i*B.i + j*B.j; }
	void operator=( Vec2i B ) { i = B.i; j = B.j; }
	void operator+=( Vec2i B ) { i += B.i; j += B.j; }
	void operator-=( Vec2i B ) { i -= B.i; j -= B.j; }
	void operator*=( int s ) { i *= s; j *= s; }
	void operator/=( int s ) { i /= s; j /= s; }
	bool operator==( Vec2i B ) { return i == B.i && j == B.j; }
	bool operator!=( Vec2i B ) { return i != B.i || j != B.j; }
	int Length() { return sqrt( float(i*i) + float(j*j) ); }
	const vec2f ScreenToWorldSpace() const;
	void worldToScreenSpace(vec2f world);

};

#endif // VEC2I_H
