#include "../gWinGl4.h"

void gWinGl4::visit(camera* _camera)
{	
	//srsly is that all?
	m_viewMat = _camera->getView();
}