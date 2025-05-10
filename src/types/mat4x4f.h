#pragma once

// #TODO: This file does not follow current conventions. Fix this.

#include "vec3f.h"

class mat4x4f
{
public:
   mat4x4f();
   mat4x4f(float _elem[][4]);
   mat4x4f(float _11, float _12, float _13, float _14,
	   float _21, float _22, float _23, float _24,
	   float _31, float _32, float _33, float _34,
	   float _41, float _42, float _43, float _44);
   mat4x4f(float newElem[16]); // OpenGL - glGetMatrix
 
   void perspective(float fovy, float aspect, float zNear, float zFar);
   mat4x4f transpose();
   
   vec3f get_position() const { return {elem[3][0], elem[3][1], elem[3][2]}; }
   
   void translate(float X, float Y, float Z);
   void scale(float X, float Y, float Z);
   void rotate(float _x, float _y, float z);
   void rotateX(float fAngle);
   void rotateY(float fAngle);
   void rotateZ(float fAngle);

   static mat4x4f rotated(float x, float y, float z);
   static mat4x4f translated(float x, float y, float z);
   static mat4x4f scaled(float x, float y, float z);

   void dumpMat4x4f(char * s = NULL);

   void lookAt(const vec3f & vFrom,
	       const vec3f & vTo,
	       const vec3f & vUp);

   inline float &operator()( unsigned int Row, unsigned int Column ) { return elem[Row][Column]; }
   inline float const &operator()( unsigned int Row, unsigned int Column ) const { return elem[Row][Column]; }
   float* asSingleArray(); // WARNING! Delete the memory returned from this function!

   float elem[4][4];
   static const mat4x4f IDENTITY;
   static const mat4x4f NULLMATRIX;
};

mat4x4f operator * ( const mat4x4f & M1, const mat4x4f & M2 );
vec3f operator * ( const mat4x4f & M, const vec3f & V );

inline mat4x4f operator - ( const mat4x4f & M )
{
   return mat4x4f(-M(0,0),-M(0,1),-M(0,2),-M(0,3),
		  -M(1,0),-M(1,1),-M(1,2),-M(1,3),
		  -M(2,0),-M(2,1),-M(2,2),-M(2,3),
		  -M(3,0),-M(3,1),-M(3,2),-M(3,3));
}

inline mat4x4f operator - ( const mat4x4f & M1, const mat4x4f & M2 )
{
   return mat4x4f(M1(0,0)-M2(0,0),M1(0,1)-M2(0,1),M1(0,2)-M2(0,2),M1(0,3)-M2(0,3),
		  M1(1,0)-M2(1,0),M1(1,1)-M2(1,1),M1(1,2)-M2(1,2),M1(1,3)-M2(1,3),
		  M1(2,0)-M2(2,0),M1(2,1)-M2(2,1),M1(2,2)-M2(2,2),M1(2,3)-M2(2,3),
		  M1(3,0)-M2(3,0),M1(3,1)-M2(3,1),M1(3,2)-M2(3,2),M1(3,3)-M2(3,3));
}

inline mat4x4f operator + ( const mat4x4f & M1, const mat4x4f & M2 )
{
   return mat4x4f(M1(0,0)+M2(0,0),M1(0,1)+M2(0,1),M1(0,2)+M2(0,2),M1(0,3)+M2(0,3),
		  M1(1,0)+M2(1,0),M1(1,1)+M2(1,1),M1(1,2)+M2(1,2),M1(1,3)+M2(1,3),
		  M1(2,0)+M2(2,0),M1(2,1)+M2(2,1),M1(2,2)+M2(2,2),M1(2,3)+M2(2,3),
		  M1(3,0)+M2(3,0),M1(3,1)+M2(3,1),M1(3,2)+M2(3,2),M1(3,3)+M2(3,3));
}

inline mat4x4f operator * ( const mat4x4f & M, const float & s )
{
   return mat4x4f(M(0,0) * s,M(0,1) * s,M(0,2) * s,M(0,3) * s,
		  M(1,0) * s,M(1,1) * s,M(1,2) * s,M(1,3) * s,
		  M(2,0) * s,M(2,1) * s,M(2,2) * s,M(2,3) * s,
		  M(3,0) * s,M(3,1) * s,M(3,2) * s,M(3,3) * s);
}

inline mat4x4f operator * ( const float & s, const mat4x4f & M )
{
   return mat4x4f(M(0,0) * s,M(0,1) * s,M(0,2) * s,M(0,3) * s,
		  M(1,0) * s,M(1,1) * s,M(1,2) * s,M(1,3) * s,
		  M(2,0) * s,M(2,1) * s,M(2,2) * s,M(2,3) * s,
		  M(3,0) * s,M(3,1) * s,M(3,2) * s,M(3,3) * s);
}

inline float deg2rad(float const degrees)
{
   const float pi = float(3.1415926535897932384626433832795);
   return degrees * (pi / float(180));
}

inline mat4x4f transposed( mat4x4f const & M )
{
   mat4x4f ret;
   for(int j = 0; j < 4; j++)
   {
      for(int i = 0; i < 4; i++)
      {
	 ret(i,j) = M(j,i);
      }
   }
   return ret;
}
