#ifndef MODEL_H
#define MODEL_H

#include <string.h>

class model
{

	public:

		enum type
		{
			e_milkshape = 0,
			e_totalTypes
		};

		struct mesh
		{
			int m_materialIndex;
			int m_numTriangles;
			int *m_pTriangleIndices;
		};
		struct material
		{
			float m_ambient[4], m_diffuse[4], m_specular[4], m_emissive[4];
			float m_shininess;
			unsigned int m_texture; //This was a GLuint
			char *m_pTextureFilename;
		};
		struct triangle
		{
			float m_vertexNormals[3][3];
			float m_s[3], m_t[3];
			int m_vertexIndices[3];
		};
		struct vertex
		{
			char m_boneID;	// for skeletal animation.
			float m_location[3];
		};

		type getType(void){return m_type;}
		int getNumMeshes(void){return m_numMeshes;}
		int getNumMaterials(void){return m_numMaterials;}
		int getNumTriangles(void){return m_numTriangles;}
		int getNumVertices(void){return m_numVertices;}

		mesh* getMeshes(void){return m_pMeshes;}
		material* getMaterials(void){return m_pMaterials;}
		triangle* getTriangles(void){return m_pTriangles;}
		vertex* getVertices(void){return m_pVertices;}



	public:

		model();
		~model();
		virtual bool loadModelData( const char *filename ) = 0;
		//void draw();
		//void reloadTextures();

	protected:

		type m_type;
	
		int m_numMeshes; //meshes used.
		mesh* m_pMeshes;

		int m_numMaterials; //materials used.
		material* m_pMaterials;

		int m_numTriangles; //triangles used.
		triangle* m_pTriangles;

		int m_numVertices; //vertices used.
		vertex* m_pVertices;
};

#endif//MODEL_H
