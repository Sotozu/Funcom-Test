#include "globalConstants.inc"
#include "stereo.inc"

RWTexture2D<float> output : register( u0 );

Texture2D depthMap : register( t0 );
sampler pointSampler : register( s0 );


[numthreads (16, 16, 1)]
void main_cs( uint3 threadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex, uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID )
{        
  float2 uv = threadID.xy * backBufferDimensions.zw * 2.f;
  float2 offset = backBufferDimensions.zw;
  
  float4 ds = depthMap.GatherRed( pointSampler, uv + offset, int2( 0, 0 ) ).x;
  float r = 0;
  #ifdef MIN
   
    r = min( ds.x, min( ds.y, min( ds.z, ds.w ) ) );
  
  #elif defined( MAX )
  
    r = max( ds.x, max( ds.y, max( ds.z, ds.w ) ) );
    
  #elif defined( AVG )
  
    r = dot( ds.xyzw, 0.25f );
  
  #endif
  
  output[ threadID.xy ] = r;
}
