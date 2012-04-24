#include "../gWinGl4.h"

void gWinGl4::visit(sprite* _sprite)
{
	//do some sort of rendery business here.
	loadTexture(_sprite->m_fileName);
	mat4x4f rotation; rotation.IDENTITY;
	mat4x4f translation; translation.IDENTITY;

	_sprite->m_orientation.createMatrix(&rotation);
	//26 is a random distance away from the camera.
	translation.translate(_sprite->m_position.i, _sprite->m_position.j, -26);
	m_modelMat = m_modelMat*rotation;
	m_modelMat = m_modelMat*translation;

	int projMatID = glGetUniformLocation(m_shaders["texture"]->id(), "projMat"); // Get the location of our projection matrix in the shader
	int viewMatID = glGetUniformLocation(m_shaders["texture"]->id(), "viewMat"); // Get the location of our view matrix in the shader
	int modelMatID = glGetUniformLocation(m_shaders["texture"]->id(), "modelMat"); // Get the location of our model matrix in the shader
	int myTextureID = glGetUniformLocation(m_shaders["texture"]->id(), "myTexture");
	
	m_shaders["texture"]->bind();
		glUniformMatrix4fv(projMatID, 1, GL_FALSE, &m_projMat.elem[0][0]); // Send our projection matrix to the shader
		glUniformMatrix4fv(viewMatID, 1, GL_FALSE, &m_viewMat.elem[0][0]); // Send our view matrix to the shader
		glUniformMatrix4fv(modelMatID, 1, GL_FALSE, &m_modelMat.elem[0][0]); // Send our model matrix to the shader
		glUniform1i(myTextureID, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textures[_sprite->m_fileName].second);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindVertexArray(m_vaos["square"]);
		//Draw Arrays doesn't take ibo's into account?
		//Index buffer objects kick ass...
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			//draw elements is for IBOs me thinks.
			//glDrawElements(GL_TRIANGLE_STRIP, 4, GL_FLOAT, 0);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	m_shaders["texture"]->unbind();

	m_modelMat = m_modelMat.IDENTITY;
}