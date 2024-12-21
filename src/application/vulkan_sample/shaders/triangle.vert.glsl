#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set=0, binding = 0) uniform UniformBufferObject
{
   // limitation of 16 objects per pass / pipeline
   mat4 model[16];
   mat4 view;
   mat4 proj;
} ubo;

layout( push_constant ) uniform constants
{
   uint id;
} perdraw;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec2 in_uv;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_color;
layout(location = 3) out vec2 out_uv;
layout(location = 4) out vec3 out_light;

void main()
{
   mat4 lmvp = ubo.proj * ubo.view * ubo.model[1];
   mat4 mvp = ubo.proj * ubo.view * ubo.model[perdraw.id];
   mat4 modelrot = ubo.model[perdraw.id];
   modelrot[3] = vec4(0,0,0,1);
   mat4 viewrot = ubo.view;

   // this probably needs to be inverse.
   viewrot[3] = vec4(0,0,0,1);
   mat4 projrot = ubo.proj;
   projrot[3] = vec4(0,0,0,1);
   //
   gl_Position =  mvp * vec4(in_position, 1.0);
   out_position = vec3(gl_Position);
   out_normal = vec3(projrot * viewrot * modelrot * vec4(in_normal, 1.0));
   out_color = in_color;
   out_uv = in_uv;
   // note: hack to give me control over a light position in the scene
   out_light = vec3(lmvp*vec4(0,0,0,1));
}
