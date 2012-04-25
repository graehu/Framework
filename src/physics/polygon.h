#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include "../graphics/renderable/iRenderable.h"

class vec3f;

//this class should probably sit inside of the
//rigid body class.

class polygon : public iRenderable
{
public:
	polygon(){};
	~polygon(){};

	//this bool will become a vec3f
	vec3f collideSAT(polygon* _polygon);
	void render(iRenderVisitor* _renderer){_renderer->visit(this);}
	std::vector<vec3f> m_vertices;

	//this needs offsets.

protected:

private:
};
#endif//POLYGON_H