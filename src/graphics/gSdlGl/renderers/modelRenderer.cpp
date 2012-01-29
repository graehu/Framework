#include "../gSdlGl.h"
#include "../../resources/model/modelFormats/milkshape/milkshapeModel.h"


void gSdlGl::visit(object3D* _object3D) 
{
	model* l_model;
	if(_object3D->getModelID() < m_models.size() && _object3D->getModelID() > -1)
	{
		l_model = m_models[_object3D->getModelID()];
	}
	else
	{
		//attempt to load based on filename.
		l_model = new milkshapeModel();
		if(!l_model->loadModelData(_object3D->getFilename()))
		{
			delete l_model;
			return;
		}
		else
		{
			loadModelTextures(l_model);
			m_models.push_back(l_model);
			_object3D->setModelID(m_models.size()-1);
		}

	}

	GLboolean texEnabled = glIsEnabled( GL_TEXTURE_2D );

	// Draw by group
	for ( int i = 0; i < l_model->getNumMeshes(); i++ )
	{
		int materialIndex = l_model->getMeshes()[i].m_materialIndex;
		if ( materialIndex >= 0 )
		{
			glMaterialfv( GL_FRONT, GL_AMBIENT, l_model->getMaterials()[materialIndex].m_ambient );
			glMaterialfv( GL_FRONT, GL_DIFFUSE, l_model->getMaterials()[materialIndex].m_diffuse );
			glMaterialfv( GL_FRONT, GL_SPECULAR, l_model->getMaterials()[materialIndex].m_specular );
			glMaterialfv( GL_FRONT, GL_EMISSION, l_model->getMaterials()[materialIndex].m_emissive );
			glMaterialf( GL_FRONT, GL_SHININESS, l_model->getMaterials()[materialIndex].m_shininess );

			if ( l_model->getMaterials()[materialIndex].m_texture > 0 )
			{
				glBindTexture(GL_TEXTURE_2D, l_model->getMaterials()[materialIndex].m_texture);
				glEnable( GL_TEXTURE_2D );
			}
			else
				glDisable( GL_TEXTURE_2D );
		}
		else
		{
			// Material properties?
			glDisable( GL_TEXTURE_2D );
		}

		glBegin( GL_TRIANGLES );
		{
			for ( int j = 0; j < l_model->getMeshes()[i].m_numTriangles; j++ )
			{
				int triangleIndex = l_model->getMeshes()[i].m_pTriangleIndices[j];
				const model::triangle* pTri = &(l_model->getTriangles()[triangleIndex]);

				for ( int k = 0; k < 3; k++ )
				{
					int index = pTri->m_vertexIndices[k];

					glNormal3fv( pTri->m_vertexNormals[k] );
					glTexCoord2f( pTri->m_s[k], pTri->m_t[k] );
					glVertex3fv( l_model->getVertices()[index].m_location );
				}
			}
		}
		glEnd();
	}

	if (texEnabled)
		glEnable( GL_TEXTURE_2D );
	else
		glDisable( GL_TEXTURE_2D );
}


GLuint loadTexture(char* string)
{
	GLuint texture;			// This is a handle to our texture object
	SDL_Surface *surface;	// This surface will tell us the details of the image
	GLenum texture_format;
	GLint  nOfColors;
	 
	if ((surface = SDL_LoadBMP(string)))
	{ 
		// Check that the image's width is a power of 2
		if ( (surface->w & (surface->w - 1)) != 0 )
		{
			printf("warning: image.bmp's width is not a power of 2\n");
		}
		
		// Also check if the height is a power of 2
		if ( (surface->h & (surface->h - 1)) != 0 ) 
		{
			printf("warning: image.bmp's height is not a power of 2\n");
		}
	 
	        // get the number of channels in the SDL surface
	        nOfColors = surface->format->BytesPerPixel;

	        if (nOfColors == 4)     // contains an alpha channel
	        {
	                if (surface->format->Rmask == 0x000000ff)
	                        texture_format = GL_RGBA;
	                else
                	        texture_format = GL_BGRA;
	        } 
		else if (nOfColors == 3)     // no alpha channel
	        {
	                if (surface->format->Rmask == 0x000000ff)
	                        texture_format = GL_RGB;
	                else
	                        texture_format = GL_BGR;
		} 
		else 
		{
	                printf("warning: the image is not truecolor..  this will probably break\n");
	                // this error should not go unhandled
	        }
	        
		// Have OpenGL generate a texture object handle for us
		glGenTextures( 1, &texture );
	 
		// Bind the texture object
		glBindTexture( GL_TEXTURE_2D, texture );
	 
		// Set the texture's stretching properties
	        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
 
		// Edit the texture object's image data using the information SDL_Surface gives us
		glTexImage2D	(
				GL_TEXTURE_2D, 
				0, 
				nOfColors, 
				surface->w, 
				surface->h, 
				0,
	            texture_format, 
				GL_UNSIGNED_BYTE, 
				surface->pixels
				);
	} 
	else 
	{
		printf("SDL could not load image.bmp: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}    
 
	// Free the SDL_Surface only if it was successfully created
	if ( surface ) 
	{ 
		SDL_FreeSurface( surface );
		return texture;
	}
	return 1;
}

void gSdlGl::loadModelTextures(model* _model)
{
	for(int i = 0; i < _model->getNumMaterials(); i++)
		if(strlen(_model->getMaterials()[i].m_pTextureFilename) > 0)
		{
			 _model->getMaterials()[i].m_texture = loadTexture(_model->getMaterials()[i].m_pTextureFilename);
		}
		else
		{
			 _model->getMaterials()[i].m_texture = 0;
		}
}