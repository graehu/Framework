#ifndef POLYGON_H
#define POLYGON_H

#include <vector>

class vec3f;

//this class should probably sit inside of the
//rigid body class.

class polygon
{
public:
	polygon(){};
	~polygon(){};

	//this bool will become a vec3f
	bool collideSAT(polygon* _polygon);
	std::vector<vec3f> m_vertices;

	//this needs offsets.

protected:

private:
};
#endif//POLYGON_H