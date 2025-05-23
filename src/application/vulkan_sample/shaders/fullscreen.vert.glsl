#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set=0, binding = 0) uniform UniformBufferObject
{
   // limitation of 16 objects per pass / pipeline
   // note: this must match DefaultUniforms, fix the 128 magic number
   mat4 model[128];
   mat4 view;
   mat4 proj;
   vec3 light;
   float light_intensity;
   vec3 cam_pos;
   uint shademode;
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
layout(location = 6) out mat4 out_modelrot;
layout(location = 10) out uint out_shademode;

void main()
{
   mat4 mvp = ubo.proj * ubo.view * ubo.model[perdraw.id];
   mat4 modelrot = ubo.model[perdraw.id];
   modelrot[3] = vec4(0,0,0,1);
   out_modelrot = modelrot;
   mat4 viewrot = ubo.view;

   viewrot[3] = vec4(0,0,0,1);
   mat4 projrot = ubo.proj;
   projrot[3] = vec4(0,0,0,1);

   // gl_Position =  vec4(in_position, 1.0);//mvp * vec4(in_position, 1.0); // screenspace position
   out_uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
   gl_Position = vec4(out_uv * 2.0f + -1.0f, 0.0f, 1.0f);
   out_position = in_position;//vec3(ubo.model[perdraw.id]*vec4(in_position, 1.0)); // world space position
   // out_normal = normalize(vec3(projrot * viewrot * modelrot * vec4(in_normal, 1.0))); // screenspace normals.
   // todo: might not need this normalize
   out_normal = normalize(vec3(modelrot * vec4(in_normal, 1.0))); // world space normals
   out_color = in_color;
   // out_uv = in_uv;
   
   out_light = vec4(ubo.light, 1.0); // worldspace light
   out_light.w = ubo.light_intensity;
   out_view_pos = ubo.cam_pos;
   out_shademode = ubo.shademode;
}
