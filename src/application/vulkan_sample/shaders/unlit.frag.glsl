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
   vec4 col_1 = texture(sampler2D(albedo, tex_sampler), in_uv);
   vec4 col_2 = texture(sampler2D(roughness, tex_sampler), in_uv);
   if(col_1 == vec4(0)) out_color = col_2;
   else if(col_2 == vec4(0)) out_color = col_1;
   else out_color = mix(col_2, col_1, col_1.a);
   if (out_color == vec4(0)) discard;
}
