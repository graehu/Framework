#include "quaternion.h"


//ctors

quaternion::quaternion()
{
	i = j = k = 0.0f;
	w = 1.0f;
}
quaternion::quaternion(float _angle, vec3f _axis)
{
	const float a = _angle * 0.5f;
	const float s = (float) sin(a);
	const float c = (float) cos(a);

	w = c;
	i = _axis.i * s;
	j = _axis.j * s;
	k = _axis.k * s;
}

float& quaternion::operator [](int _i)
{
	assert(_i>=0);
	assert(_i<=2);
	return *(&w+_i);
}


quaternion::quaternion(mat4x4f _matrix)
{
	// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
	// article "Quaternion Calculus and Fast Animation".

	const float trace = _matrix(0, 0) + _matrix(1, 1) + _matrix(2, 2);

	if (trace>0)
	{
		// |w| > 1/2, may as well choose w > 1/2

		float root = sqrt(trace + 1.0f);  // 2w
		w = 0.5f * root;
		root = 0.5f / root;  // 1/(4w)
		i = (_matrix(2, 1)-_matrix(1,2)) * root;
		j = (_matrix(0,2)-_matrix(2, 0)) * root;
		k = (_matrix(1,0)-_matrix(0,1)) * root;
	}
	else
	{
		// |w| <= 1/2

		static int next[3] = { 2, 3, 1 };
		
		int x = 1;
		if (_matrix(1,1)>_matrix(0,0))  x = 2;
		if (_matrix(2,2)>_matrix(i,i)) x = 3;
		int y = next[x];
		int z = next[y];
		
		float root = sqrt(_matrix(x,x)-_matrix(y,y)-_matrix(z,z) + 1.0f);
		float *quaternion[3] = { &i, &j, &k };
		*quaternion[x] = 0.5f * root;
		root = 0.5f / root;
		w = (_matrix(z,y)-_matrix(y,z))*root;
		*quaternion[y] = (_matrix(y,x)+_matrix(x,y))*root;
		*quaternion[z] = (_matrix(z,x)+_matrix(x,z))*root;
	}
}
void quaternion::identity()
{
	w = 1;
	i = 0;
	j = 0;
	k = 0;
}
void quaternion::createFromAxisAngle(float x, float y, float z, float degrees)
{
	// First we want to convert the degrees to radians 
	// since the angle is assumed to be in radians
	float angle = float((degrees / 180.0f) * PI);

	// Here we calculate the sin( theta / 2) once for optimization
	float result = (float)sin( angle / 2.0f );
		
	// Calcualte the w value by cos( theta / 2 )
	w = (float)cos( angle / 2.0f );

	// Calculate the x, y and z of the quaternion
	i = float(x * result);
	j = float(y * result);
	k = float(z * result);
}

void quaternion::createMatrix(mat4x4f *pMatrix)
{
	// Make sure the matrix has allocated memory to store the rotation data
	if(!pMatrix) return;

	//transposed from opengl

	// First row
	pMatrix->elem[0][0] = 1.0f - 2.0f * ( j * j + k * k ); 
	pMatrix->elem[0][1] = 2.0f * (i * j + k * w);
	pMatrix->elem[0][2] = 2.0f * (i * k - j * w);
	pMatrix->elem[0][3] = 0.0f;  

	// Second row
	pMatrix->elem[1][0] = 2.0f * ( i * j - k * w );  
	pMatrix->elem[1][1] = 1.0f - 2.0f * ( i * i + k * k ); 
	pMatrix->elem[1][2] = 2.0f * (k * j + i * w );  
	pMatrix->elem[1][3] = 0.0f;  

	// Third row
	pMatrix->elem[2][0] = 2.0f * ( i * k + j * w );
	pMatrix->elem[2][1] = 2.0f * ( j * k - i * w );
	pMatrix->elem[2][2] = 1.0f - 2.0f * ( i * i + j * j );  
	pMatrix->elem[2][3] = 0.0f;  

	// Fourth row
	pMatrix->elem[3][0] = 0;  
	pMatrix->elem[3][1] = 0;  
	pMatrix->elem[3][2] = 0;  
	pMatrix->elem[3][3] = 1.0f;

	// Now pMatrix[] is a 4x4 homogeneous matrix that can be applied to an OpenGL Matrix
}



float quaternion::length() {return sqrt(w*w + i*i + j*j + k*k);}

void quaternion::normalise()
{
	const float length = this->length();

	if (length == 0)
	{
		w = 1;
		i = 0;
		j = 0;
		k = 0;
	}
	else
	{
		float inv = 1.0f / length;
		i = i * inv;
		j = j * inv;
		k = k * inv;
		w = w * inv;
	}
}



void quaternion::operator +=(quaternion q)
{
	w+=q.w;
	i+=q.i;
	j+=q.j;
	k+=q.k;
}
quaternion quaternion::operator *(quaternion q) const
{
	quaternion r;

	r.w = w*q.w - i*q.i - j*q.j - k*q.k;
	r.i = w*q.i + i*q.w + j*q.k - k*q.j;
	r.j = w*q.j + j*q.w + k*q.i - i*q.k;
	r.k = w*q.k + k*q.w + i*q.j - j*q.i;
	
	return(r);
}
quaternion quaternion::operator +(quaternion q) const
{
	return quaternion(w+q.w, i+q.i, j+q.j, k+q.k);
}

quaternion quaternion::operator *(float s) const
{
	return quaternion(w*s, i*s, j*s, k*s);
}
//dtor
quaternion::~quaternion()
{
}
