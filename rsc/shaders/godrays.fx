#include "globalConstants.inc"
#include "shadows.inc"
#include "stereo.inc"
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

sampler pointClampSampler : register( s0 );
sampler linearClampSampler : register( s1 );
Texture2D depthBuffer : register( t0 );
#if BLUR_PASS
	Texture2D godraysBuffer : register( t1 );
#else
	Texture3D volLightBuffer : register( t1 );
#endif

VertexOutput main_vs( in VertexInput v )
{
	VertexOutput o;
	
	o.hpos = v.pos;
	o.uv = v.uv;

	return o;
} 

#if !BLUR_PASS

float4 main_ps( in VertexOutput f ) : SV_Target0
{	
	float depth = depthBuffer.SampleLevel( pointClampSampler, f.uv, 0 ).x;
	
#ifdef STEREO
	uint eyeIndex = getStereoConstantIndexFromUV( f.uv );
	float2 rescaledEyeUVs = getStereoRescaledUVs( f.uv.xy, eyeIndex );
	
	float4 HCoords = float4(2.f * rescaledEyeUVs.x - 1.f, 1.f - 2.f * rescaledEyeUVs.y, depth, 1.f );
	float4 WorldPos = mul( matInvProjViewStereo[ eyeIndex ], HCoords );
	WorldPos = WorldPos / WorldPos.w;
	
	float3 camPos = cameraPositionStereo[ eyeIndex ].xyz;
#else
	float4 HCoords = float4(2.f * f.uv.x - 1.f, 1.f - 2.f * f.uv.y, depth, 1.f );
	float4 WorldPos = mul( matInvViewProj, HCoords );
	WorldPos = WorldPos / WorldPos.w;
	
	float3 camPos = cameraPosition.xyz;
#endif

	float3 endPos = WorldPos.xyz;
	float3 camToPoint = WorldPos.xyz - camPos.xyz;
	float dist = length( camToPoint );
	float3 rayDirection = normalize( camToPoint );
	
	//rescale to max shadow dist if needed
	dist = min( dist, cascadeDistances.z );
	endPos = camPos.xyz + rayDirection * dist;
	
	#define NUM_GODRAY_STEPS 96
	uint actualNumStepsDS = saturate( ( dist * 1.5f ) / cascadeDistances.z ) * NUM_GODRAY_STEPS;
	
	float shadowFactor = getRayMarchedShadowAccumulation( camPos.xyz, endPos, actualNumStepsDS );
	
	float coeff = distance( endPos, camPos.xyz ) / cascadeDistances.z;
	float intensity = lightDirGodrayIntensity.w * max( dot( -lightDirGodrayIntensity.xyz, rayDirection ) * 0.5f + 0.5f, 0.5f );
	float godrays = shadowFactor * intensity * coeff;
	
	//local lights
	#define NUM_STEPS 60
	float3 colAcc = 0.f;
	
#ifdef LLIGHT_SCATTER
	
#ifdef STEREO
	float3 initCoords = float3( rescaledEyeUVs.xy, 0.f );
	float3 endCoords = float3( rescaledEyeUVs.xy, 1.f );
#else
	float3 initCoords = float3( f.uv.xy, 0.f );
	float3 endCoords = float3( f.uv.xy, 1.f );
#endif
	float3 step = ( endCoords - initCoords ) / NUM_STEPS;
	float actualNumSteps = min( dist / 0.3f, NUM_STEPS );
	[loop]
	for( float j = 0.f; j < actualNumSteps; j++ )
	{
		float scaledOffset = j / NUM_STEPS;
		float falloff = 1.f - scaledOffset * scaledOffset;
		colAcc += volLightBuffer.SampleLevel( linearClampSampler, initCoords + ( j * step ), 0 ).xyz * falloff;
	}
	colAcc /= NUM_STEPS * 2.f;
#endif //LLIGHT_SCATTER
	
  float linZApprox = saturate( distance( WorldPos, camPos ) / 255.f );

	return float4( colAcc.xyz + godrays.xxx, linZApprox );
}

#else

cbuffer cb : register( b1 )
{
  float4 godrayResInvRes;
};

float4 main_ps( in VertexOutput f ) : SV_Target0
{	
	float4 godrays = godraysBuffer.SampleLevel( linearClampSampler, f.uv, 0 );
	
    //return float4( godrays.xyz, 1.f );

  float4 acc = godrays;
  float div = 1.f;
  [unroll]
  for( float i = -1.5f; i <= 1.5f; i+=1.5f )
  {
    [unroll]
    for( float j = -1.5f; j <= 1.5f; j+=1.5f )
    {
      float4 s = godraysBuffer.SampleLevel( linearClampSampler, f.uv + float2( i, j ) * godrayResInvRes.zw, 0 );
      float coeff = abs( godrays.w - s.w ) > 0.0039 ? 0.f : 1.f;
      acc.xyz += s.xyz * coeff;
      div += coeff;
    }
  }
  acc.xyz /= div;
  acc.w = ( div < 10.f );
  return acc;
  

  /*
	float4 sR0 = godraysBuffer.GatherRed( linearClampSampler, f.uv, int2( -1, -1 ) );
	float4 sR1 = godraysBuffer.GatherRed( linearClampSampler, f.uv, int2( -1, 1 ) );
	float4 sR2 = godraysBuffer.GatherRed( linearClampSampler, f.uv, int2( 1, -1 ) );
	float4 sR3 = godraysBuffer.GatherRed( linearClampSampler, f.uv, int2( 1, 1 ) );
	
	float4 sG0 = godraysBuffer.GatherGreen( linearClampSampler, f.uv, int2( -1, -1 ) );
	float4 sG1 = godraysBuffer.GatherGreen( linearClampSampler, f.uv, int2( -1, 1 ) );
	float4 sG2 = godraysBuffer.GatherGreen( linearClampSampler, f.uv, int2( 1, -1 ) );
	float4 sG3 = godraysBuffer.GatherGreen( linearClampSampler, f.uv, int2( 1, 1 ) );
	
	float4 sB0 = godraysBuffer.GatherBlue( linearClampSampler, f.uv, int2( -1, 1 ) );
	float4 sB1 = godraysBuffer.GatherBlue( linearClampSampler, f.uv, int2( -1, -1 ) );
	float4 sB2 = godraysBuffer.GatherBlue( linearClampSampler, f.uv, int2( 1, 1 ) );
	float4 sB3 = godraysBuffer.GatherBlue( linearClampSampler, f.uv, int2( 1, -1 ) );
	
	float one = 1.f;
	float r = ( godrays.x + dot( sR0.xyzw, one.xxxx ) + dot( sR1.xyzw, one.xxxx ) + dot( sR2.xyzw, one.xxxx ) + dot( sR3.xyzw, one.xxxx ) ) * 0.05882;
	float g = ( godrays.y + dot( sR0.xyzw, one.xxxx ) + dot( sG1.xyzw, one.xxxx ) + dot( sG2.xyzw, one.xxxx ) + dot( sG3.xyzw, one.xxxx ) ) * 0.05882;
	float b = ( godrays.z + dot( sR0.xyzw, one.xxxx ) + dot( sB1.xyzw, one.xxxx ) + dot( sB2.xyzw, one.xxxx ) + dot( sB3.xyzw, one.xxxx ) ) * 0.05882;
	
	return float4( r, g, b, 1.f );*/
}

#endif
