#version 450
#extension GL_ARB_separate_shader_objects : enable
// this allows #include.
#extension GL_ARB_shading_language_include : enable
#define PI 3.14159265358979323846

layout(set=0, binding = 1) uniform sampler tex_sampler;
layout(set=1, binding = 0) uniform texture2D albedo_tex;
layout(set=1, binding = 1) uniform texture2D roughness_tex;
layout(set=1, binding = 2) uniform texture2D normal_tex;
layout(set=1, binding = 3) uniform texture2D ao_tex;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec2 in_uv;
layout(location = 4) flat in vec4 in_light_pos;
layout(location = 5) flat in vec3 in_view_pos;
layout(location = 6) flat in mat4 in_modelrot;
layout(location = 10) flat in uint in_shademode;

layout(location = 0) out vec4 out_color;

struct Lights
{
   vec3 position;
   vec3 diffuse;
   float intensity;
};

// k term is for image based versus direct lighting
// ibl = (rougness*rougness)*0.5f
// direct = ((roughness+1)^2)*0.125
// below are the DGF terms of cook-torrance without
// multiplying the k term. Roughness is multiplied
// for direct lights in the used functions.

// float GeometrySchlickGGX(float NdotV, float k)
// {
//     float nom   = NdotV;
//     float denom = NdotV * (1.0 - k) + k;
	
//     return nom / denom;
// }

// Used for self shadowing.
// float GeometrySmith(vec3 N, vec3 V, vec3 L, float k) 
// {
//     float NdotV = max(dot(N, V), 0.0);
//     float NdotL = max(dot(N, L), 0.0);
//     float ggx1 = GeometrySchlickGGX(NdotV, k);
//     float ggx2 = GeometrySchlickGGX(NdotL, k);
	
//     return ggx1 * ggx2;
// }

// used for the specular component
// float DistributionGGX(vec3 N, vec3 H, float a)
// {
//     float a2     = a*a;
//     float NdotH  = max(dot(N, H), 0.0);
//     float NdotH2 = NdotH*NdotH;
	
//     float nom    = a2;
//     float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
//     denom        = PI * denom * denom;
	
//     return nom / denom;
// }

// fersnel / surface refraction
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
   return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// diffuse reflectance / linear scatter
float lambert(vec3 norm, vec3 lightdir)
{
   norm = normalize(norm);
   lightdir = normalize(lightdir);
   return max(dot(norm, lightdir), 0);
}

// specular reflectance / normal fall off function.
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
   float a      = roughness*roughness;
   float a2     = a*a;
   float NdotH  = max(dot(N, H), 0.0);
   float NdotH2 = NdotH*NdotH;
	
   float num   = a2;
   float denom = (NdotH2 * (a2 - 1.0) + 1.0);
   denom = PI * denom * denom;
	
   return num / denom;
}

// geometry self shadowing
float GeometrySchlickGGX(float NdotV, float roughness)
{
   float r = (roughness + 1.0);
   float k = (r*r) / 8.0;

   float num   = NdotV;
   float denom = NdotV * (1.0 - k) + k;
	
   return num / denom;
}

// geometry self shadowing
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
   float NdotV = max(dot(N, V), 0.0);
   float NdotL = max(dot(N, L), 0.0);
   float ggx2  = GeometrySchlickGGX(NdotV, roughness);
   float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
   return ggx1 * ggx2;
}
#define BETTER_METHOD 1
// not using this atm, it makes fresnel look worse.
mat3 CalculateTBN( vec3 N, vec3 p, vec2 uv )
{
   // get edge vectors of the pixel triangle
   #if BETTER_METHOD
   vec3 dp1 = dFdx( p );
   vec3 dp2 = dFdy( p );
   vec2 duv1 = dFdx( uv );
   vec2 duv2 = dFdy( uv );

   // solve the linear system
   vec3 dp2perp = cross( dp2, N );
   vec3 dp1perp = cross( N, dp1 );
   vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
   vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

   // construct a scale-invariant frame 
   float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
   return mat3( T * invmax, B * invmax, N );
   #else
   // simpler method below
   // note: this method can flip normal directions.
   // ----: might be ok for geometry with normals in one hemisphere.
   
   // compute tangent T and bitangent B
   vec3 Q1 = dFdx(p);
   vec3 Q2 = dFdy(p);
   vec2 st1 = dFdx(uv);
   vec2 st2 = dFdy(uv);
	
   vec3 T = normalize(Q1*st2.t - Q2*st1.t);
   vec3 B = normalize(-Q1*st2.s + Q2*st1.s);
	
   // the transpose of texture-to-eye space matrix
   return mat3(T, B, N);
   #endif
}

// note: there's something wrong with the fall off of the normal distribution.
// ----: the normal distribution / light decay is strong at specific angles / directions.

// do I care about specular ibl and diffuse irradience.
// not right now?
void main()
{
   vec3 world_normal = normalize(in_normal);
   vec3 view_direction = normalize(in_view_pos - in_position);
   vec3 tex_normal = texture(sampler2D(normal_tex, tex_sampler), in_uv).rgb;
   vec4 albedo = texture(sampler2D(albedo_tex, tex_sampler), in_uv);
   if(albedo.a == 0) discard;
   // convert srgb to linear
   albedo = pow(albedo, vec4(2.2));
   // gltf metallic and roughness are packed in one texture
   vec3 metallicRoughness = texture(sampler2D(roughness_tex, tex_sampler), in_uv).rgb;
   float metallic = metallicRoughness.b;
   float roughness = metallicRoughness.g;
   
   // currently binding grey by default, test out bindnig white.
   float ao = texture(sampler2D(ao_tex, tex_sampler), in_uv).r;
   // todo: refractive index, probably ought to be on a uniform.
   vec3 RI = vec3(0.045);
   RI = mix(RI, albedo.rgb, metallic);
    
   Lights light;
   light.position = in_light_pos.xyz;
   // todo: this isn't bound
   light.diffuse = vec3(1.0);
   light.intensity = in_light_pos.w;
	           
   vec3 Lo = vec3(0.0);
   // loop for however many lights you have, I have 1.
   // for(int i = 0; i < 4; ++i) 
   // {
   // calculate per-light radiance
   vec3 position = in_position;
   vec3 light_direction = normalize(light.position-position);

   // todo: remove this branch nicely.
   if (tex_normal != vec3(0.0))
   {
      // this should move us from tangent space, to world space
      mat3 tbn = CalculateTBN(world_normal, position, in_uv);
      world_normal = normalize(tbn*tex_normal);
   }
   
   // todo: check the math here? original halfway vector is:
   // vec3 halfway = normalize(in_view_pos + light.position);
   vec3 halfway = normalize(view_direction + light_direction);
   float dist = length(light.position - position);
   float attenuation = 1.0 / (dist * dist);
   vec3 radiance = (light.intensity * light.diffuse * attenuation);
        
   // cook-torrance brdf - Bidirectional Reflectance Distribution Function
   float normal_distrib = DistributionGGX(world_normal, halfway, roughness);
   float self_occlusion = GeometrySmith(world_normal, view_direction, light_direction, roughness);
   vec3 fresnel = fresnelSchlick(max(dot(halfway, view_direction), 0.0), RI);

   vec3 kS = fresnel;
   vec3 kD = vec3(1.0) - kS;
   kD *= 1.0 - metallic;
   
   vec3 numerator = normal_distrib * self_occlusion * fresnel;
   float denominator = 4.0 * max(dot(world_normal, view_direction), 0.0) + max(dot(world_normal, light_direction), 0.0) + 0.0001;
   // lambert / diffuse reflectance
   // This is also very aggressive, not sure why. Should this be light dir?
   vec3 specular = numerator / denominator;
            
   // add to outgoing radiance Lo
   float NdotL = max(dot(world_normal, light_direction), 0.0);
   Lo += (kD * vec3(albedo) / PI + specular) * radiance * NdotL; 
   // }
   
   // todo: bind ambient intensity + diffuse.
   vec3 am_light = vec3(0.01);
   vec3 ambient = am_light * vec3(albedo) * ao;
   vec3 color = ambient + Lo;
    
   color = color / (color + vec3(1.0));
   // convert from linear to srgb
   color = pow(color, vec3(1.0/2.2));

   switch(in_shademode)
   {
      case 0: out_color = vec4(color, clamp(albedo.a,0.0,1.0)); break;
      //  Debugging modes
      case 1: out_color = vec4(in_uv, 1.0, 1.0); break;        // show uvs
      case 2: out_color = pow(albedo, vec4(1.0/2.2)); break;   // show texture albedo
      case 3: out_color = vec4(abs(in_normal), 1.0); break;    // show vertex normals
      case 4: out_color = vec4(tex_normal, 1.0); break;        // show texture normals
      case 5: out_color = vec4(abs(world_normal), 1.0); break; // show world normals
      case 6: out_color = vec4(vec3(roughness), 1.0); break;   // show texture roughness
      case 7: out_color = vec4(vec3(metallic), 1.0); break;    // show texture metallic
      case 8: out_color = vec4(vec3(ao), 1.0);  break;         // show ao
      case 9: out_color = vec4(vec3(albedo.a), 1.0); break;    // show alpha

      default: out_color = vec4(color, 1.0); break;
   }
   
   // debugging

   // normals
   // ------
   // out_color = vec4(abs(world_normal), 1.0); // show normal
   // out_color = vec4(world_normal, 1.0); // show normals
   // out_color = vec4(abs(world_normal.x)); // show normals
   // out_color = vec4(abs(world_normal.y)); // show normals
   // out_color = vec4(abs(world_normal.z)); // show normals

   // pbr lighting
   // ------------
   // out_color = vec4(normal_distrib);
   // out_color = vec4(self_occlusion);
   // out_color = vec4(specular, 1.0);
   // out_color = vec4(denominator);
   // out_color = vec4(fresnel, 1.0);
   // out_color = vec4(light_direction, 1.0); // show lightdir
   // out_color = vec4(abs(light.position)*.1, 1.0); // show light.position
   // out_color = vec4(vec3(lambert(world_normal, light_direction)), 1.0); // show lambert
   // out_color = vec4(abs(in_view_pos)*0.1, 1.0); // show view_pos
   // out_color = vec4(abs(halfway), 1.0); // show halway
   // out_color = vec4(vec3(attenuation), 1.0); // show attenuation
   // out_color = vec4(vec3(radiance), 1.0); // show radiance
   
   // texturing
   // ---------
   // out_color = vec4(in_uv, 1.0, 1.0); // show uvs
   // out_color = vec4(vec3(roughness), 1.0); // show roughness
   // out_color = vec4(vec3(metallic), 1.0); // show metallic
   // out_color = vec4(vec3(ao), 1.0); // show ao

   // misc
   // ---
   // out_color = vec4(in_color, 1.0); // show colors
   // out_color = vec4(abs(position), 1.0); // show position


}
