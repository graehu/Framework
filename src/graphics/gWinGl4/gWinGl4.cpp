#include "gWinGl4.h"

#include <GL/gl.h>
#include <windows.h>

//#pragma comment(lib, "glew32.lib")
//#pragma comment(lib, "opengl32.lib")

int gWinGl4::init(void)
{

	///Going to have to get the window height somehow, probably through windows api.
	
	//Perspective calculations.
	HWND hwnd = GetForegroundWindow();
	m_hdc = GetDC(hwnd);
	RECT rect;
	GetWindowRect(hwnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	m_projMat.perspective(60.0f, (float)width / (float)height, 0.1f, 100.f);
	////////////////////////////////////////
	float squareVerts[12] = 
	{
		-1.0f,-1.0f, 0.0f, // bot left
		 -1.0f, 1.0f, 0.0f, //top left
		 1.0f,-1.0f, 0.0f, // bot right
		 1.0f, 1.0f, 0.0f // top right
	};
	GLuint squareVID;
	glGenBuffers(1, &squareVID);
	glBindBuffer(GL_ARRAY_BUFFER, squareVID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(squareVerts),&squareVerts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	////////////////////////////////////////
	float squareNorms[12] = 
	{
		0.0f, 0.0f, 1.0f, // bot left
		 0.0f, 0.0f, 1.0f, //top left
		 0.0f, 0.0f, 1.0f, // bot right
		 0.0f, 0.0f, 1.0f // top right
	};
	GLuint squareNID;
	glGenBuffers(1, &squareNID);
	glBindBuffer(GL_ARRAY_BUFFER, squareNID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(squareNorms),&squareNorms, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	////////////////////////////////////////
	float squareTexCoords[8] =  //these should probably be 2d.
	{
		 0.0f, 0.0f,  // bot left
		 0.0f, 1.0f,  //top left
		 1.0f, 0.0f,  // bot right
		 1.0f, 1.0f  // top right
	};
	GLuint squareTID;
	glGenBuffers(1, &squareTID);
	glBindBuffer(GL_ARRAY_BUFFER, squareTID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(squareTexCoords),&squareTexCoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	////////////////////////////////////////

	m_vaos["square"] = 0;
	
	glGenVertexArrays(1, &m_vaos["square"]);
	glBindVertexArray(m_vaos["square"]);
		glBindBuffer(GL_ARRAY_BUFFER, squareVID);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0); // Set up our vertex attributes pointer
			glEnableVertexAttribArray(0); //Enable Vertex Atribute array 0 (so positions)
		glBindBuffer(GL_ARRAY_BUFFER, squareNID);
			glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1); //Enable Vertex Atribute array 1 (so normals)
		glBindBuffer(GL_ARRAY_BUFFER, squareTID);
			glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(2); //Enable Vertex Atribute array 2 (so texCoords)
	glBindVertexArray(0);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		assert(false);
		throw -1;
	}
	m_shaders["texture"] = new shader("assets/GLSL/texture.vs","assets/GLSL/texture.gs","assets/GLSL/texture.fs");
	return 0;

} //initializing gl and what not for whatever window system is implamented

int gWinGl4::render(void)
{

	SwapBuffers(m_hdc); // Swap buffers so we can see our rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers
	return 0;

} //this'll have flipping the buffers
int gWinGl4::shutdown(void)
{
	return 0;

} //shutsdown the graphics engine
int gWinGl4::update(void)
{
	return 0;

}//this is currently useless.
iRenderVisitor* gWinGl4::getRenderer(void)
{
	return this;
}//passes back the renderer

void gWinGl4::loadTexture(char* _fileName) //assumes 2d texture.
{///This can only load freaking bmps.
	if(m_textures.find(_fileName) != m_textures.end()) //texture is already loaded.
		return;

	//make an abstract image class.
	bitmap* image = new bitmap(_fileName);
	m_textures[_fileName] = std::pair<bitmap*,GLuint>(image, 0);
	int happy = image->getRedVal(16,16);

	glGenTextures(1, &m_textures[_fileName].second);
	glBindTexture(GL_TEXTURE_2D, m_textures[_fileName].second);
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB8,
			image->getWidth(),
			image->getHeight(),
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			image->getBitmapData());
	glBindTexture(GL_TEXTURE_2D, 0);
}
graphics* graphics::graphicsFactory(void)
{
	return new gWinGl4;
}