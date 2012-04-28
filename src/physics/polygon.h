#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include "../graphics/renderable/iRenderable.h"

class vec3f;

///A Generic polygon shape class, used for colidable areas.
class polygon : public iRenderable
{
public:
	polygon(){};
	~polygon(){};

	///Colides this polygon against another, returning the minimum translation vector to seperate them.
	vec3f collideSAT(polygon* _polygon); 
	///Renders this polygon, wireframe
	void render(iRenderVisitor* _renderer){_renderer->visit(this);}
	///Stores the vertices that make up the body.
	std::vector<vec3f> m_vertices;

protected:

private:
};
#endif//POLYGON_H