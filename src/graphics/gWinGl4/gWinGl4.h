#ifndef GWINGL4_H
#define GWINGL4_H

#include "../graphics.h"
#include "bitmap/bitmap.h"
#include "../../types/Mat4x4f.h"
#include "../renderable/sprite/sprite.h"
#include "../camera/camera.h"
#include "../../physics/polygon.h"
#include "shader/shader.h"
#include <windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/gl.h>
#include <map>

class gWinGl4 : public graphics, public iRenderVisitor
{
public:
	gWinGl4(){}
	~gWinGl4(){}

	int init(void); //initializing gl and what not for whatever window system is implamented
	int render(void); //this'll have flipping the buffers
	int shutdown(void); //shutsdown the graphics engine
	int update(void); //this is currently useless.
	iRenderVisitor* getRenderer(void); //passes back the renderer

	//visitor section

	void visit(sprite* _sprite);
	void visit(camera* _camera);
	void visit(polygon* _poly);

	//utilities
	void loadTexture(char* _fileName); // texture

protected:

	HDC m_hdc;

	std::map<char*, std::pair<bitmap*, GLuint> > m_textures; //filename, pair<bitmap data, GPUmem address>
	std::map<char*, GLuint> m_vaos; //These are vaos. Might want to store vbo's and ibos here too?
	std::map<char*, shader*> m_shaders;
		
	mat4x4f m_projMat; // projectionMatrix; // Store the projection matrix
	mat4x4f m_viewMat; // viewMatrix; // Store the view matrix
	mat4x4f m_modelMat; // modelMatrix; // Store the model matrix
};

#endif//GWINGL4_H