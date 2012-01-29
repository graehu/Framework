#ifndef OBJECT3D_H
#define OBJECT3D_H

#include "../iRenderable.h"

class object3D : public iRenderable
{
public:
	object3D(char* _filename):m_filename(_filename), m_modelID(-1){}
	~object3D(){}
	void render(iRenderVisitor* _renderer){_renderer->visit(this);}
	void setFilename(char* _filename){m_filename = _filename; m_modelID = -1;}
	char* getFilename(void){return m_filename;}
	void setModelID(int _ID){m_modelID = _ID;}
	int getModelID(void){return m_modelID;}

protected:
	char* m_filename; // The model file.
	int m_modelID;    // The model's internal id, after loading.
private:

};

#endif//OBJECT3D_H