namespace pbr_sl
{
#define PI 3.14159265358979323846
// consider ParamaterBlock.
layout(set = 0, binding = 1) SamplerState sampler_state;
layout(set = 1, binding = 0) Texture2D albedo_tex;
layout(set = 1, binding = 1) Texture2D roughness_tex;
layout(set = 1, binding = 2) Texture2D normal_tex;
layout(set = 1, binding = 3) Texture2D ao_tex;

struct PSInput
{
    float3 position;
    float3 normal;
    float3 colour;
    float2 uv;
    nointerpolation float4 light_pos;
    nointerpolation float3 view_pos;
    nointerpolation float4x4 modelrot;
    nointerpolation uint shademode;
};
struct PSOut {
    float4 color;
};

struct Lights
{
   float3 position;
   float3 diffuse;
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
// float GeometrySmith(float3 N, float3 V, float3 L, float k) 
// {
//     float NdotV = max(dot(N, V), 0.0);
//     float NdotL = max(dot(N, L), 0.0);
//     float ggx1 = GeometrySchlickGGX(NdotV, k);
//     float ggx2 = GeometrySchlickGGX(NdotL, k);
	
//     return ggx1 * ggx2;
// }

// used for the specular component
// float DistributionGGX(float3 N, float3 H, float a)
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
float3 fresnelSchlick(float cosTheta, float3 F0)
{
   return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// diffuse reflectance / linear scatter
float lambert(float3 norm, float3 lightdir)
{
   norm = normalize(norm);
   lightdir = normalize(lightdir);
   return max(dot(norm, lightdir), 0);
}

// specular reflectance / normal fall off function.
float DistributionGGX(float3 N, float3 H, float roughness)
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
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
   float NdotV = max(dot(N, V), 0.0);
   float NdotL = max(dot(N, L), 0.0);
   float ggx2  = GeometrySchlickGGX(NdotV, roughness);
   float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
   return ggx1 * ggx2;
}
#define BETTER_METHOD 1
// not using this atm, it makes fresnel look worse.
float3x3 CalculateTBN( float3 N, float3 p, float2 uv )
{
   // get edge vectors of the pixel triangle
   #if BETTER_METHOD
   float3 dp1 = ddx( p );
   float3 dp2 = ddy( p );
   float2 duv1 = ddx( uv );
   float2 duv2 = ddy( uv );

   // solve the linear system
   float3 dp2perp = cross( dp2, N );
   float3 dp1perp = cross( N, dp1 );
   float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
   float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

   // construct a scale-invariant frame 
   float invmax = rsqrt( max( dot(T,T), dot(B,B) ) );
   return float3x3( T * invmax, B * invmax, N );
   #else
   // simpler method below
   // note: this method can flip normal directions.
   // ----: might be ok for geometry with normals in one hemisphere.
   
   // compute tangent T and bitangent B
   float3 Q1 = ddx(p);
   float3 Q2 = ddy(p);
   float2 st1 = ddx(uv);
   float2 st2 = ddy(uv);
	
   float3 T = normalize(Q1*st2.t - Q2*st1.t);
   float3 B = normalize(-Q1*st2.s + Q2*st1.s);
	
   // the transpose of texture-to-eye space matrix
   return float3x3(T, B, N);
   #endif
}

// note: there's something wrong with the fall off of the normal distribution.
// ----: the normal distribution / light decay is strong at specific angles / directions.

// do I care about specular ibl and diffuse irradience.
// not right now?
[shader("pixel")]
PSOut main(PSInput input)
{
   PSOut output;
   float3 world_normal = normalize(input.normal);
   float3 view_direction = normalize(input.view_pos - input.position);
   float4 tex_normal = normal_tex.Sample(sampler_state, input.uv);
   float4 albedo = albedo_tex.Sample(sampler_state, input.uv);
   if(albedo.a == 0) discard;
   // convert srgb to linear
   albedo = pow(albedo, float4(2.2));
   // gltf metallic and roughness are packed in one texture
   float3 metallicRoughness = roughness_tex.Sample(sampler_state, input.uv).rgb;
   float metallic = metallicRoughness.b;
   float roughness = metallicRoughness.g;
   
   // currently binding grey by default, test out bindnig white.
   float ao = ao_tex.Sample(sampler_state, input.uv).r;
   // todo: refractive index, probably ought to be on a uniform.
   float3 RI = float3(0.045f);
   RI = lerp(RI, albedo.rgb, metallic);
    
   Lights light;
   light.position = input.light_pos.xyz;
   // todo: this isn't bound
   light.diffuse = float3(1.0);
   light.intensity = input.light_pos.w;
	           
   float3 Lo = float3(0.0);
   // loop for however many lights you have, I have 1.
   // for(int i = 0; i < 4; ++i) 
   // {
   // calculate per-light radiance
   float3 position = input.position;
   float3 light_direction = normalize(light.position-position);

   // todo: remove this branch nicely.
   if (!tex_normal.equals(float4(0.0)))
   {
      // this should move us from tangent space, to world space
      float3x3 tbn = CalculateTBN(world_normal, position, input.uv.rg);
      world_normal = normalize(mul(tex_normal.rgb, tbn));
   }
   
   // todo: check the math here? original halfway vector is:
   // float3 halfway = normalize(input.view_pos + light.position);
   float3 halfway = normalize(view_direction + light_direction);
   float dist = length(light.position - position);
   float attenuation = 1.0 / (dist * dist);
   float3 radiance = (light.intensity * light.diffuse * attenuation);
        
   // cook-torrance brdf - Bidirectional Reflectance Distribution Function
   float normal_distrib = DistributionGGX(world_normal, halfway, roughness);
   float self_occlusion = GeometrySmith(world_normal, view_direction, light_direction, roughness);
   float3 fresnel = fresnelSchlick(max(dot(halfway, view_direction), 0.0), RI);

   float3 kS = fresnel;
   float3 kD = float3(1.0) - kS;
   kD *= 1.0 - metallic;
   
   float3 numerator = normal_distrib * self_occlusion * fresnel;
   float denominator = 4.0 * max(dot(world_normal, view_direction), 0.0) + max(dot(world_normal, light_direction), 0.0) + 0.0001;
   // lambert / diffuse reflectance
   // This is also very aggressive, not sure why. Should this be light dir?
   float3 specular = numerator / denominator;
            
   // add to outgoing radiance Lo
   float NdotL = max(dot(world_normal, light_direction), 0.0);
   Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL; 
   // }
   
   // todo: bind ambient intensity + diffuse.
   float3 am_light = float3(0.01);
   float3 ambient = am_light * albedo.rgb * ao;
   float3 color = ambient + Lo;
    
   color = color / (color + float3(1.0));
   // convert from linear to srgb
   color = pow(color, float3(1.0/2.2));

   switch(input.shademode)
   {
      case 0: output.color = float4(color, clamp(albedo.a,0.0,1.0)); break;
      //  Debugging modes
      case 1: output.color = float4(input.uv, 1.0, 1.0); break;        // show uvs
      case 2: output.color = pow(albedo, float4(1.0/2.2)); break;   // show texture albedo
      case 3: output.color = float4(abs(input.normal), 1.0); break;    // show vertex normals
      case 4: output.color = float4(tex_normal.rgb, 1.0); break;        // show texture normals
      case 5: output.color = float4(abs(world_normal), 1.0); break; // show world normals
      case 6: output.color = float4(float3(roughness), 1.0); break;   // show texture roughness
      case 7: output.color = float4(float3(metallic), 1.0); break;    // show texture metallic
      case 8: output.color = float4(float3(ao), 1.0);  break;         // show ao
      case 9: output.color = float4(float3(albedo.a), 1.0); break;    // show alpha

      default: output.color = float4(color, 1.0); break;
   }
   
   // debugging

   // normals
   // ------
   // output.color = float4(abs(world_normal), 1.0); // show normal
   // output.color = float4(world_normal, 1.0); // show normals
   // output.color = float4(abs(world_normal.x)); // show normals
   // output.color = float4(abs(world_normal.y)); // show normals
   // output.color = float4(abs(world_normal.z)); // show normals

   // pbr lighting
   // ------------
   // output.color = float4(normal_distrib);
   // output.color = float4(self_occlusion);
   // output.color = float4(specular, 1.0);
   // output.color = float4(denominator);
   // output.color = float4(fresnel, 1.0);
   // output.color = float4(light_direction, 1.0); // show lightdir
   // output.color = float4(abs(light.position)*.1, 1.0); // show light.position
   // output.color = float4(float3(lambert(world_normal, light_direction)), 1.0); // show lambert
   // output.color = float4(abs(input.view_pos)*0.1, 1.0); // show view_pos
   // output.color = float4(abs(halfway), 1.0); // show halway
   // output.color = float4(float3(attenuation), 1.0); // show attenuation
   // output.color = float4(float3(radiance), 1.0); // show radiance
   
   // texturing
   // ---------
   // output.color = float4(input.uv, 1.0, 1.0); // show uvs
   // output.color = float4(float3(roughness), 1.0); // show roughness
   // output.color = float4(float3(metallic), 1.0); // show metallic
   // output.color = float4(float3(ao), 1.0); // show ao

   // misc
   // ---
   // output.color = float4(input.color, 1.0); // show colors
   // output.color = float4(abs(position), 1.0); // show position
   return output;
}
}