#include "quaternion.h"

quaternion::quaternion()
{
	m_i = m_j = m_k = 0.0f;
	m_w = 1.0f;
}

quaternion::~quaternion()
{

}

void quaternion::createFromAxisAngle(float x, float y, float z, float degrees)
{
	// First we want to convert the degrees to radians 
	// since the angle is assumed to be in radians
	float angle = float((degrees / 180.0f) * PI);

	// Here we calculate the sin( theta / 2) once for optimization
	float result = (float)sin( angle / 2.0f );
		
	// Calcualte the w value by cos( theta / 2 )
	m_w = (float)cos( angle / 2.0f );

	// Calculate the x, y and z of the quaternion
	m_i = float(x * result);
	m_j = float(y * result);
	m_k = float(z * result);
}

void quaternion::createMatrix(Mat4x4f *pMatrix)
{
	// Make sure the matrix has allocated memory to store the rotation data
	if(!pMatrix) return;

	//transposed from opengl

	// First row
	pMatrix->elem[0][0] = 1.0f - 2.0f * ( m_j * m_j + m_k * m_k ); 
	pMatrix->elem[0][1] = 2.0f * (m_i * m_j + m_k * m_w);
	pMatrix->elem[0][2] = 2.0f * (m_i * m_k - m_j * m_w);
	pMatrix->elem[0][3] = 0.0f;  

	// Second row
	pMatrix->elem[1][0] = 2.0f * ( m_i * m_j - m_k * m_w );  
	pMatrix->elem[1][1] = 1.0f - 2.0f * ( m_i * m_i + m_k * m_k ); 
	pMatrix->elem[1][2] = 2.0f * (m_k * m_j + m_i * m_w );  
	pMatrix->elem[1][3] = 0.0f;  

	// Third row
	pMatrix->elem[2][0] = 2.0f * ( m_i * m_k + m_j * m_w );
	pMatrix->elem[2][1] = 2.0f * ( m_j * m_k - m_i * m_w );
	pMatrix->elem[2][2] = 1.0f - 2.0f * ( m_i * m_i + m_j * m_j );  
	pMatrix->elem[2][3] = 0.0f;  

	// Fourth row
	pMatrix->elem[3][0] = 0;  
	pMatrix->elem[3][1] = 0;  
	pMatrix->elem[3][2] = 0;  
	pMatrix->elem[3][3] = 1.0f;

	// Now pMatrix[] is a 4x4 homogeneous matrix that can be applied to an OpenGL Matrix
}

quaternion quaternion::operator *(quaternion q)
{
	quaternion r;

	r.m_w = m_w*q.m_w - m_i*q.m_i - m_j*q.m_j - m_k*q.m_k;
	r.m_i = m_w*q.m_i + m_i*q.m_w + m_j*q.m_k - m_k*q.m_j;
	r.m_j = m_w*q.m_j + m_j*q.m_w + m_k*q.m_i - m_i*q.m_k;
	r.m_k = m_w*q.m_k + m_k*q.m_w + m_i*q.m_j - m_j*q.m_i;
	
	return(r);
}
