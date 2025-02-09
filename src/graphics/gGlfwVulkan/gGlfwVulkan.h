#pragma once

#include "../graphics.h"
#include "../../types/mat4x4f.h"
struct VkInstance_T;

class gGlfwVulkan : public graphics, public iRenderVisitor
{
public:
   //graphics section
   int update() override;
   int render() override;

   int init() override;
   int shutdown() override;

   bool register_shader(fw::hash::string name, const char* path, fw::shader::type type) override;
   bool register_pass(fw::hash::string name) override;

   iRenderVisitor* getRenderer(void) override;

   //visitor section
   void visit(class physics::collider::polygon* _poly) override;
   void visit(class camera* _camera) override;
   void visit(fw::Mesh* _mesh) override;
   void visit(fw::Light* _mesh) override;
};
