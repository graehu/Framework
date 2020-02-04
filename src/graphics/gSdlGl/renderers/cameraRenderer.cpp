#include "../gSdlGl.h"
#include "../../camera/camera.h"



void gSdlGl::visit(camera* _camera)
{
  m_viewMat = _camera->getView();
}
