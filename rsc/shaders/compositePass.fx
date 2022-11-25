#include "globalConstants.inc"
#include "utils.inc"

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

sampler DiffuseSampler : register(s0);
Texture2D<float4> DiffuseTex : register(t0);

sampler LightSampler : register(s1);
Texture2D<float4> LightTex : register(t1);

sampler SpecSampler : register(s2);
Texture2D<float4> SpecTex : register(t2);

sampler PointSampler : register(s3);
Texture2D DepthTex : register(t3);

sampler LinearSampler : register(s4);
Texture2D GodrayTex : register(t4);

cbuffer cb : register( b1 )
{
  float4 godrayResInvRes;
};

VertexOutput main_vs( in VertexInput v )
{
	VertexOutput o;
	
	o.hpos = v.pos;
	o.uv = v.uv;

	return o;
} 

float4 main_ps( in VertexOutput f, in float4 vpPos : SV_Position ) : SV_Target0
{
  /*
    all other buffers are at full resolution, so just sample them
  */
	float4 color = DiffuseTex.Sample( DiffuseSampler, f.uv ); 
	float4 lightBuffer = LightTex.Sample( LightSampler, f.uv );
	float4 specBuffer = SpecTex.Sample( SpecSampler, f.uv );
	

  /*
    upscale and sample godrays
  */
#define SMOOTHSTEP_UPSCALING 0
#define DEPTH_AWARE_UPSCALING 1

#if SMOOTHSTEP_UPSCALING

  float2 relGRSize = backBufferDimensions.xy / godrayResInvRes.xy;
  float2 baseGrUv = floor( f.uv * godrayResInvRes.xy ) * godrayResInvRes.zw;
  float2 nextGrUv = baseGrUv + godrayResInvRes.zw;
  float2 coeff = ( f.uv - baseGrUv ) / ( nextGrUv - baseGrUv );
  float2 upscaledUv = baseGrUv + smoothstep( 0.f, 1.f, coeff ) * godrayResInvRes.zw;
  float4 godrays = GodrayTex.Sample( LinearSampler, upscaledUv );

#elif DEPTH_AWARE_UPSCALING

  float4 godrays = GodrayTex.Sample( LinearSampler, f.uv );
  [branch]
  if( godrays.w > 0.f )
  {
    float depth = getCameraDepth( DepthTex.SampleLevel( PointSampler, f.uv, 0 ).x );

    float2 GrUv = ( floor( f.uv * godrayResInvRes.xy ) + 0.5f ) * godrayResInvRes.zw;
  
    float2 grUVs[ 5 ];
    grUVs[ 0 ] =  GrUv;
    grUVs[ 1 ] =  GrUv + float2( -godrayResInvRes.z, 0.f );
    grUVs[ 2 ] =  GrUv + float2( godrayResInvRes.z, 0.f );
    grUVs[ 3 ] =  GrUv + float2( 0.f, -godrayResInvRes.w );
    grUVs[ 4 ] =  GrUv + float2( 0.f, godrayResInvRes.w );

    float grDepths[ 5 ];
    grDepths[ 0 ] = getCameraDepth( DepthTex.SampleLevel( LinearSampler, GrUv, 0 ).x );
    grDepths[ 1 ] = getCameraDepth( DepthTex.SampleLevel( LinearSampler, grUVs[ 1 ], 0 ).x );
    grDepths[ 2 ] = getCameraDepth( DepthTex.SampleLevel( LinearSampler, grUVs[ 2 ], 0 ).x );
    grDepths[ 3 ] = getCameraDepth( DepthTex.SampleLevel( LinearSampler, grUVs[ 3 ], 0 ).x );
    grDepths[ 4 ] = getCameraDepth( DepthTex.SampleLevel( LinearSampler, grUVs[ 4 ], 0 ).x );

    float minDist = 9999.f;
    uint index = 0;
    [unroll]
    for( uint i = 0; i < 5; i++ )
    {
      float deltaDepth = abs( depth - grDepths[ i ] );
      if( deltaDepth < minDist )
      {
        index = i;
        minDist = deltaDepth;
      }
    }

    godrays = GodrayTex.SampleLevel( LinearSampler, grUVs[ index ], 0 );
  }
#else

  float4 godrays = GodrayTex.Sample( LinearSampler, f.uv );

#endif


	return float4( color.xyz * lightBuffer.xyz + specBuffer.xyz * color.a + godrays.xyz, 1.f );
}
