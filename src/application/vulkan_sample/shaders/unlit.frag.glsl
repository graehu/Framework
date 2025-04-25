#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set=0, binding = 1) uniform sampler tex_sampler;
layout(set=1, binding = 0) uniform texture2D albedo;
layout(set=1, binding = 1) uniform texture2D roughness;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

void main()
{
   out_color = texture(sampler2D(albedo, tex_sampler), in_uv);
   out_color = mix(texture(sampler2D(roughness, tex_sampler), in_uv), out_color, 0.5);
   // out_color = mix(out_color, vec4(in_uv,0,1), 0.5);
}
