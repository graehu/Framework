#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform texture2D albedo;
layout(binding = 2) uniform texture2D roughness;
layout(binding = 3) uniform sampler tex_sampler;

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

void main()
{
   out_color = texture(sampler2D(albedo, tex_sampler), in_uv);
}
