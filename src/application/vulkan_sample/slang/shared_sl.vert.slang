namespace shared_sl
{
struct UniformBufferObject
{
   // limitation of 16 objects per pass / pipeline
   // note: this must match DefaultUniforms, fix the 128 magic number
   float4x4 model[128];
   float4x4 view;
   float4x4 proj;
   float3 light;
   float light_intensity;
   float3 cam_pos;
   uint shademode;
};

ParameterBlock<UniformBufferObject> ubo;

struct VSInput
{
   float3 position;
   float3 normal;
   float3 color;
   float2 uv;
};

struct VSOutput
{
   float3 position;
   float3 normal;
   float3 color;
   float2 uv;
   float4 light;
   float3 view_pos;
   float4x4 modelrot;
   uint shademode;
   float4 gl_Position : SV_Position;
};


[shader("vertex")]
VSOutput main(VSInput input, uniform uint perdraw)
{
   VSOutput output;
   float4x4 modelrot = ubo.model[perdraw];
   modelrot[3] = float4(0,0,0,1);
   output.modelrot = modelrot;
   float4x4 viewrot = ubo.view;
   

   viewrot[3] = float4(0,0,0,1);
   float4x4 projrot = ubo.proj;
   projrot[3] = float4(0,0,0,1);

   float4x4 mvp = mul(ubo.model[perdraw], mul(ubo.proj, ubo.view));

   output.gl_Position = mul(mvp, float4(input.position, 1.0));                 // screenspace position
   output.position = mul(ubo.model[perdraw], float4(input.position, 1.0)).xyz; // world space position
   // todo: might not need this normalize
   output.normal = normalize(mul(modelrot, float4(input.normal, 1.0)).xyz); // world space normals
   output.color = input.color;
   output.uv = input.uv;
   
   output.light = float4(ubo.light, 1.0); // worldspace light
   output.light.w = ubo.light_intensity;
   output.view_pos = ubo.cam_pos;
   output.shademode = ubo.shademode;
   return output;
}
}