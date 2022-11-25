#include "globalConstants.inc"

RWTexture2D<float4> sceneBuffer : register( u0 );

Texture2D depthMap : register( t0 );
sampler DefaultSampler : register(s0);

struct ParticleData
{
  float4 pos;
  float4 col;
  float4 acceleration;
};

StructuredBuffer<ParticleData> ParticleArray : register( t1 );
StructuredBuffer<uint> ParticleSystemData : register( t2 );


[numthreads (64, 1, 1)]
void main_cs( uint3 threadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex, uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID )
{

  float2 uv = 0.f;
  if( threadID.x < ParticleSystemData[0] )
  {
    float4 worldPos = float4( ParticleArray[threadID.x].pos.xyz, 1.f );
    float4 screenPos = mul( matViewProj, worldPos );
    screenPos /= screenPos.w;
    uv = screenPos.xy * backBufferDimensions.xy;

    sceneBuffer[uint2(500, threadID.x)] = float4(1, 0 ,1, 1);//ParticleArray[threadID.x].col;
  }

}
