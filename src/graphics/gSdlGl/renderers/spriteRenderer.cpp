#include "../gSdlGl.h"

// this is how a an image is blited to a surface
// This doesn't appear to have any of the sprite culling.
// unimportant for the moment.
bool happy = false;
std::pair<SDL_Surface, GLuint>* image;
void gSdlGl::visit(sprite* _sprite) //rendering a sprite :)
{

	if(happy == false)
	{
		loadImage(_sprite->m_fileName);
		happy = true;
		image = &m_images[_sprite->m_fileName];
	}

	glLoadIdentity();
	glTranslatef(_sprite->m_x, _sprite->m_y, -512);
	
	glBindTexture(GL_TEXTURE_2D, image->second);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDisable(GL_DEPTH_TEST);

	glBegin(GL_QUADS);                      // Draw A Quad
		glTexCoord2f (0.0, 1.0);
		glVertex3f(-(image->first.w/2), (image->first.h/2), 0.0f);              // Top Left
		glTexCoord2f (1.0, 1.0);
		glVertex3f( (image->first.w/2), (image->first.h/2), 0.0f);              // Top Right
		glTexCoord2f (1.0, 0.0);
		glVertex3f( (image->first.w/2),-(image->first.h/2), 0.0f);              // Bottom Right
		glTexCoord2f (0.0, 0.0);
		glVertex3f(-(image->first.w/2),-(image->first.h/2), 0.0f);              // Bottom Left
    glEnd();
	// Done Drawing The Quad


	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
	glLoadIdentity();//*/
}
