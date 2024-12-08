#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D albedo;
layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

void main()
{
   out_color = texture(albedo, in_uv);
   out_color = mix(out_color, vec4(in_uv,0,1), 0.5);
   // out_color = mix(out_color, vec4(1,0,1,1), 0.5);
   // out_color = vec4(in_uv,0,1);
   // out_color = vec4(in_uv,0,1);
}
