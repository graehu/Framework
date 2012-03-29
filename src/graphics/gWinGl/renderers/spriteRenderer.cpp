#include "../gWinGl.h"

void gWinGl::visit(sprite* _sprite)
{
	//do some sort of rendery business here.

	loadTexture(_sprite->m_fileName);

	bitmap* image = m_textures[_sprite->m_fileName].first;
	GLuint* imageID = &m_textures[_sprite->m_fileName].second;

	glLoadIdentity();
	glTranslatef(_sprite->m_x, _sprite->m_y, -512);
	
	glBindTexture(GL_TEXTURE_2D, *imageID);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDisable(GL_DEPTH_TEST);

	glBegin(GL_QUADS);                      // Draw A Quad
		glTexCoord2f (0.0, 1.0);
		glVertex3f(-((float)image->getWidth()/2), ((float)image->getHeight()/2), 0.0f);              // Top Left
		glTexCoord2f (1.0, 1.0);
		glVertex3f( ((float)image->getWidth()/2), ((float)image->getHeight()/2), 0.0f);              // Top Right
		glTexCoord2f (1.0, 0.0);
		glVertex3f( ((float)image->getWidth()/2),-((float)image->getHeight()/2), 0.0f);              // Bottom Right
		glTexCoord2f (0.0, 0.0);
		glVertex3f(-((float)image->getWidth()/2),-((float)image->getHeight()/2), 0.0f);              // Bottom Left
    glEnd();
	// Done Drawing The Quad


	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
	glLoadIdentity();//*/
}