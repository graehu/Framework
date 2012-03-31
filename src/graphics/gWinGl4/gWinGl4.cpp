#include "gWinGl4.h"

#include <GL/gl.h>
#include <windows.h>

//#pragma comment(lib, "glew32.lib")
//#pragma comment(lib, "opengl32.lib")

int gWinGl4::init(void)
{

	///Going to have to get the window height somehow, probably through windows api.

	HWND hwnd = GetForegroundWindow();
	m_hdc = GetDC(hwnd);
	RECT rect;
	GetWindowRect(hwnd, &rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	m_projMat.Perspective(60.0f, (float)width / (float)height, 0.1f, 100.f);
	m_viewMat.Translate(0,0,-10);

	///////glTranslatef(-1.5f,0.0f,-6.0f);
	float vert[9] = 
	{
		 -0.0f, 1.0f, 0.0f,
		1.0f,-1.0f, 0.0f,
		 -1.0f,-1.0f, 0.0f
	};
	
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
		
		glGenBuffers(1,&m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vert),&vert, GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	m_shader.init("assets/GLSL/simple.vs","assets/GLSL/simple.gs","assets/GLSL/simple.fs");

	
	return 0;

} //initializing gl and what not for whatever window system is implamented

int gWinGl4::render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers

	//these methods wont work~~~~~

	int projMatLocation = glGetUniformLocation(m_shader.id(), "projectionMatrix"); // Get the location of our projection matrix in the shader
	int viewMatLocation = glGetUniformLocation(m_shader.id(), "viewMatrix"); // Get the location of our view matrix in the shader
	int modelMatLocation = glGetUniformLocation(m_shader.id(), "modelMatrix"); // Get the location of our model matrix in the shader

	//glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	m_shader.bind();

		glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, &m_projMat.elem[0][0]); // Send our projection matrix to the shader
		glUniformMatrix4fv(viewMatLocation, 1, GL_FALSE, &m_viewMat.elem[0][0]); // Send our view matrix to the shader
		glUniformMatrix4fv(modelMatLocation, 1, GL_FALSE, &m_modelMat.elem[0][0]); // Send our model matrix to the shader

		glBindVertexArray(m_vao);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);


	m_shader.unbind();
	
	//glDrawBuffer(GL_TRIANGLES);
	//		(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(vaoID[0]); // Bind our Vertex Array Object
	//	glDrawElements(GL_TRIANGLES, (WIDTH)*(HEIGHT)*6, GL_UNSIGNED_SHORT, 0); // Draw our square
	//glBindVertexArray(0); // Unbind our Vertex Array Object	


	SwapBuffers(m_hdc); // Swap buffers so we can see our rendering
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