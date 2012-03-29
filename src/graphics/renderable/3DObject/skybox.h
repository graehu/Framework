#ifndef SKYBOX_H
#define SKYBOX_H

#include "../iRenderable.h"
#include "../../../types/vec3f.h"

class skybox : public iRenderable
{
	//in the future this class will hold references to images for all of it's sides. For now, it's hacked.
public:
	skybox(){}
	~skybox(){}
	void render(iRenderVisitor* _renderer){_renderer->visit(this);}
	vec3f pos;
};
#endif//SKYBOX_H