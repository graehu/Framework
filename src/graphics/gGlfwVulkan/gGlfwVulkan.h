#pragma once

// #include <vector>
// #include <map>

#include "../graphics.h"
#include "../../types/mat4x4f.h"
struct VkInstance_T;

class gGlfwVulkan : public graphics, public iRenderVisitor
{
 public:
  //graphics section
  int update();
  int render();

  int init();
  int shutdown();

  iRenderVisitor* getRenderer(void);

  //visitor section
  void visit(class physics::collider::polygon* _poly);
  void visit(class camera* _camera);

 protected:

  //needed for cameras
  /* mat4x4f m_projMat; // projectionMatrix; // Store the projection matrix */
  /* mat4x4f m_viewMat; // viewMatrix; // Store the view matrix */
  /* mat4x4f m_modelMat; // modelMatrix; // Store the model matrix */

 private:
  // void CreateInstance(void);
  // bool CheckValidationLayerSupport(void);
  // VkInstance_T* m_instance;

};
