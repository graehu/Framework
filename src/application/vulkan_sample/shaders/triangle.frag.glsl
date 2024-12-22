#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set=0, binding = 1) uniform sampler tex_sampler;
layout(set=1, binding = 0) uniform texture2D albedo;
layout(set=1, binding = 1) uniform texture2D roughness;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec2 in_uv;
layout(location = 4) flat in vec4 light_pos;

layout(location = 0) out vec4 out_color;


struct Lights
{
   vec3 position;
   vec3 diffuse;
   float intensity;
};

float lambert(vec3 norm, vec3 lightdir)
{
  norm = normalize(norm);
  lightdir = normalize(lightdir);
  return max(dot(norm, lightdir), 0);
}

void main()
{
   Lights light;
   light.position = light_pos.xyz;
   light.diffuse = vec3(1.0, 1.0, 1.0);
   light.intensity = light_pos.w;
   
   vec3 lightdir = light.position-in_position;
   vec3 result = light.diffuse * light.intensity * lambert(in_normal, lightdir);
   
   // out_color = vec4(in_normal, 1.0); // show normals
   // out_color = vec4(lightdir, 1.0); // show lightdir
   // out_color = vec4(result, 1.0); // show lambert
   // out_color = vec4(in_color, 1.0); // show colors
   // out_color = vec4(in_uv, 1.0, 1.0); // show uvs

   out_color = texture(sampler2D(albedo, tex_sampler), in_uv);
   out_color = out_color*vec4(result, 1.0);
}
