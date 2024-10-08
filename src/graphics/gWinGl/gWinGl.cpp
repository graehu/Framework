#include "gWinGl.h"

#include <cassert>

#include <windows.h>
//#pragma comment(lib, "opengl32.lib")

int gWinGl::init(void)
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
	m_viewMat.translate(0,0,-32);

	glMatrixMode(GL_PROJECTION);
		glMultMatrixf(&m_projMat.elem[0][0]);
	glMatrixMode(GL_MODELVIEW);
		glMultMatrixf(&m_viewMat.elem[0][0]);

	return 0;

} //initializing gl and what not for whatever window system is implamented

int gWinGl::render(void)
{
	SwapBuffers(m_hdc); // Swap buffers so we can see our rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers
	return 0;

} //this'll have flipping the buffers
int gWinGl::shutdown(void)
{
	return 0;

} //shutsdown the graphics engine
int gWinGl::update(void)
{
	return 0;

}//this is currently useless.
iRenderVisitor* gWinGl::getRenderer(void)
{
	return this;
}//passes back the renderer

void gWinGl::loadTexture(char* _fileName) //assumes 2d texture.
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
	return new gWinGl;
}
