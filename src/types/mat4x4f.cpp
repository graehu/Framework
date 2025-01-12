#include <memory.h>
#include <stdio.h>
#include "mat4x4f.h"

const mat4x4f mat4x4f::IDENTITY(1.0f,0.0f,0.0f,0.0f,
				0.0f,1.0f,0.0f,0.0f,
				0.0f,0.0f,1.0f,0.0f,
				0.0f,0.0f,0.0f,1.0f);

const mat4x4f mat4x4f::NULLMATRIX(0.0f,0.0f,0.0f,0.0f,
				  0.0f,0.0f,0.0f,0.0f,
				  0.0f,0.0f,0.0f,0.0f,
				  0.0f,0.0f,0.0f,0.0f);


// mat4x4f::mat4x4f(const mat4x4f & rhs)
// {
//    memcpy(elem, rhs.elem, sizeof(float) * 16);
// }

mat4x4f::mat4x4f()
{
   memcpy(elem, &IDENTITY, sizeof(float) * 16);
}

mat4x4f::mat4x4f(float _11, float _12, float _13, float _14,
		 float _21, float _22, float _23, float _24,
		 float _31, float _32, float _33, float _34,
		 float _41, float _42, float _43, float _44)
{
   elem[0][0] = _11;	elem[0][1] = _12;	elem[0][2] = _13;	elem[0][3] = _14;
   elem[1][0] = _21;	elem[1][1] = _22;	elem[1][2] = _23;	elem[1][3] = _24;
   elem[2][0] = _31;	elem[2][1] = _32;	elem[2][2] = _33;	elem[2][3] = _34;
   elem[3][0] = _41;	elem[3][1] = _42;	elem[3][2] = _43;	elem[3][3] = _44;
}

mat4x4f::mat4x4f(float _elem[][4])
{
   memcpy(elem, _elem, sizeof(float) * 16);
}

mat4x4f::mat4x4f(float newElem[16])
{
   memcpy(elem,newElem,sizeof(float) * 16);
}

mat4x4f operator*( const mat4x4f &M1, const mat4x4f &M2)
{
   mat4x4f ret;

   for(int i = 0; i < 4; i++)
   {
      for(int j = 0; j < 4; j++)
      {
	 float Value = 0;

	 for(int k = 0; k < 4; k++)
	 {
	    Value += M1(i,k) * M2(k,j);
	 }

	 ret(i,j) = Value;
      }
   }

   return ret;
}

vec3f operator * (const mat4x4f &M, const vec3f &V)
{
   vec3f ret;

   ret.i = M(0,0) * V.i + M(1,0) * V.j + M(2,0) * V.k + M(3,0);
   ret.j = M(0,1) * V.i + M(1,1) * V.j + M(2,1) * V.k + M(3,1);
   ret.k = M(0,2) * V.i + M(1,2) * V.j + M(2,2) * V.k + M(3,2);
   float w = M(0,3) * V.i + M(1,3) * V.j + M(2,3) * V.k + M(3,3);
   ret /= w;
   return ret;
}

void mat4x4f::perspective(float fovy, float aspect, float zNear, float zFar)
{
   memcpy(elem, IDENTITY.elem, sizeof(float) * 16);

   float range = tan(deg2rad(fovy / float(2))) * zNear;
   float left = -range * aspect;
   float right = range * aspect;
   float bottom = -range;
   float top = range;

   elem[0][0] = (float(2) * zNear) / (right - left);
   elem[1][1] = (float(2) * zNear) / (top - bottom);
   elem[2][2] = - (zFar + zNear) / (zFar - zNear);
   elem[2][3] = - float(1);
   elem[3][2] = - (float(2) * zFar * zNear) / (zFar - zNear);
}

mat4x4f mat4x4f::transpose()
{
   mat4x4f temp = *this;
   for(int i = 0; i < 4; i++)
   {
      for(int ii = 0; ii < 4; ii++)
      {
	 elem[i][ii] = temp.elem[ii][i];
      }
   }
   return *this;
}

void mat4x4f::translate(float X, float Y, float Z)
{
   memcpy(elem, IDENTITY.elem, sizeof(float) * 16);

   elem[3][0] = X;
   elem[3][1] = Y;
   elem[3][2] = Z;
}

void mat4x4f::scale(float X, float Y, float Z)
{
   memcpy(elem, IDENTITY.elem, sizeof(float) * 16);

   elem[0][0] = X;
   elem[1][1] = Y;
   elem[2][2] = Z;
}

void mat4x4f::rotateX(float fAngle)
{
   memcpy(elem, IDENTITY.elem, sizeof(float) * 16);
   
   float c = cos(fAngle);
   float s = sin(fAngle);
   elem[1][1] = c;
   elem[1][2] = s;
   elem[2][1] = -s;
   elem[2][2] = c;
}

void mat4x4f::rotateY(float fAngle)
{
   memcpy(elem, IDENTITY.elem, sizeof(float) * 16);
   
   float c = cos(fAngle);
   float s = sin(fAngle);
   elem[0][0] = c;
   elem[2][0] = s;
   elem[0][2] = -s;
   elem[2][2] = c;
}

void mat4x4f::rotateZ(float fAngle)
{
   memcpy(elem, IDENTITY.elem, sizeof(float) * 16);
   
   float c = cos(fAngle);
   float s = sin(fAngle);
   elem[0][0] = c;
   elem[0][1] = s;
   elem[1][0] = -s;
   elem[1][1] = c;
}

void mat4x4f::rotate(float x, float y, float z)
{
   mat4x4f rx; rx.rotateX(x);
   mat4x4f ry; ry.rotateY(y);
   mat4x4f rz; rz.rotateZ(z);
   mat4x4f ret = rx*ry*rz;
   
   memcpy(elem, ret.elem, sizeof(float) * 16);
}

mat4x4f mat4x4f::rotated(float x, float y, float z)
{
   mat4x4f ret; ret.rotate(x, y, z); return ret;
}

mat4x4f mat4x4f::translated(float x, float y, float z)
{
   mat4x4f ret; ret.translate(x, y, z); return ret;
}

mat4x4f mat4x4f::scaled(float x, float y, float z)
{
   mat4x4f ret; ret.scale(x, y, z); return ret;
}

void mat4x4f::lookAt(const vec3f & vFrom, const vec3f & vTo, const vec3f & vUp)
{
   vec3f vZ = normalise(vFrom - vTo);
   vec3f vX = normalise(vUp.cross_product(vZ));
   vec3f vY = vZ.cross_product(vX);

   elem[0][0] = vX.i;	elem[0][1] = vY.i;	elem[0][2] = vZ.i;	elem[0][3] = 0;
   elem[1][0] = vX.j;	elem[1][1] = vY.j;	elem[1][2] = vZ.j;	elem[1][3] = 0;
   elem[2][0] = vX.k;	elem[2][1] = vY.k;	elem[2][2] = vZ.k;	elem[2][3] = 0;

   elem[3][0] = -vX.dot_product3(vFrom);
   elem[3][1] = -vY.dot_product3(vFrom);
   elem[3][2] = -vZ.dot_product3(vFrom);
   elem[3][3] = 1;
}

float* mat4x4f::asSingleArray()
{
   float* a;
   a = new float[16];
   memcpy(a,elem[0],sizeof(float) * 4);
   memcpy(a+4,elem[1],sizeof(float) * 4);
   memcpy(a+8,elem[2],sizeof(float) * 4);
   memcpy(a+12,elem[3],sizeof(float) * 4);
   return a;
}

void mat4x4f::dumpMat4x4f(char * s)
{
   if(s != NULL)printf("\n%s\n",s);
   else printf("\n");
   printf("%f %f %f %f\n",   elem[0][0], elem[0][1], elem[0][2], elem[0][3]);
   printf("%f %f %f %f\n",   elem[1][0], elem[1][1], elem[1][2], elem[1][3]);
   printf("%f %f %f %f\n",   elem[2][0], elem[2][1], elem[2][2], elem[2][3]);
   printf("%f %f %f %f\n\n", elem[3][0], elem[3][1], elem[3][2], elem[3][3]);
}
