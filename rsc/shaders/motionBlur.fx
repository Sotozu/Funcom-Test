#include "globalConstants.inc"

struct VertexInput 
{
	float4 pos 		: POSITION;
	float2 uv 		: TEXCOORD0;
};

struct VertexOutput
{
	float4 hpos		: SV_Position;
	float2 uv 		: TEXCOORD0;
};

cbuffer cb1 : register( b1 )
{
  float4 intensityFramerate; //zw unused
};

sampler DiffuseSampler : register(s1);
Texture2D<float4> DiffuseTex : register(t1);

sampler DepthSampler : register(s0);
Texture2D<float4> DepthTex : register(t0);

sampler VelocitySampler : register(s2);
Texture2D<float4> VelocityTex : register(t2);

VertexOutput main_vs( in VertexInput v )
{
	VertexOutput o;
	
	o.hpos = v.pos;
	o.uv = v.uv;

	return o;
} 

float4 main_ps( in VertexOutput f ) : SV_Target0
{	
	float depth = DepthTex.Sample( DepthSampler, f.uv ).r; 

  float2 vel = VelocityTex.Sample( VelocitySampler, f.uv ).xy; 

  #define NUM_SAMPLES 16

  float4 color = DiffuseTex.Sample( DiffuseSampler, f.uv ); 

  float depthScale = saturate(pow(depth, 20.f) * 2.f);
  float framerateScale = intensityFramerate.y * 0.01666667f;
  vel *= depthScale * intensityFramerate.x * framerateScale;
  #define MAX_SCALE 0.03f
  float scaleClamp = MAX_SCALE / max( length(vel), MAX_SCALE );
  vel *= scaleClamp;

  vel /= NUM_SAMPLES;
  
  float2 texCoord = f.uv + vel;  
  float totalWeight = 1.f;
  for(uint i = 1; i < NUM_SAMPLES; ++i, texCoord += vel)  
  {  
    float weight = 1.f - (0.5f * ( i / NUM_SAMPLES ) );
    totalWeight += weight;
    float4 currentColor = DiffuseTex.Sample( DiffuseSampler, texCoord );
 
    color += currentColor;  
  }  

  float4 finalColor = color / (totalWeight + 1.f);  

	return float4(finalColor.xyz, 1.f);
}
