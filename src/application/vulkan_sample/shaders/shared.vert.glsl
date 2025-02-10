#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set=0, binding = 0) uniform UniformBufferObject
{
   // limitation of 16 objects per pass / pipeline
   mat4 model[16];
   mat4 view;
   mat4 proj;
   vec3 light;
   float light_intensity;
   vec3 cam_pos;
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
layout(location = 4) out vec4 out_light;
layout(location = 5) out vec3 out_view_pos;

void main()
{

   mat4 mvp = ubo.proj * ubo.view * ubo.model[perdraw.id];
   mat4 modelrot = ubo.model[perdraw.id];
   modelrot[3] = vec4(0,0,0,1);
   mat4 viewrot = ubo.view;

   viewrot[3] = vec4(0,0,0,1);
   mat4 projrot = ubo.proj;
   projrot[3] = vec4(0,0,0,1);

   gl_Position =  mvp * vec4(in_position, 1.0); // screenspace position
   out_position = vec3(ubo.model[perdraw.id]*vec4(in_position, 1.0)); // world space position
   // out_normal = normalize(vec3(projrot * viewrot * modelrot * vec4(in_normal, 1.0))); // screenspace normals.
   // todo: might not need this normalize
   out_normal = normalize(vec3(modelrot * vec4(in_normal, 1.0))); // world space normals
   out_color = in_color;
   out_uv = in_uv;
   
   out_light = vec4(ubo.light, 1.0); // worldspace light
   out_light.w = ubo.light_intensity;
   out_view_pos = ubo.cam_pos;
}
