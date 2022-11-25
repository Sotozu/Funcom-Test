#include "globalConstants.inc"
#include "shadows.inc"
#include "stereo.inc"

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
Texture2D depthBuffer : register( t0 );
Texture2D normalBuffer : register( t1 );

VertexOutput main_vs( in VertexInput v )
{
	VertexOutput o;
	
	o.hpos = v.pos;
	o.uv = v.uv;

	return o;
} 

float4 main_ps( in VertexOutput f ) : SV_Target0
{	
	float3 normal = normalize( normalBuffer.SampleLevel( pointClampSampler, f.uv, 0 ).xyz * 2.f - 1.f );
	float depth = depthBuffer.SampleLevel( pointClampSampler, f.uv, 0 ).x;

#ifdef STEREO
	uint eyeIndex = getStereoConstantIndexFromUV( f.uv );
	float2 rescaledEyeUVs = getStereoRescaledUVs( f.uv.xy, eyeIndex );
	
	float4 HCoords = float4( 2.f * rescaledEyeUVs.x - 1.f, 1.f - 2.f * rescaledEyeUVs.y, depth, 1.f );
	float4 WorldPos = mul( matInvProjViewStereo[ eyeIndex ], HCoords );
	WorldPos = WorldPos / WorldPos.w;
	
	float linDepth = length( WorldPos.xyz - cameraPositionStereo[ eyeIndex ].xyz );
#else
	
	float4 HCoords = float4( 2.f * f.uv.x - 1.f, 1.f - 2.f * f.uv.y, depth, 1.f );
	float4 WorldPos = mul( matInvViewProj, HCoords );
	WorldPos = WorldPos / WorldPos.w;
	
	float linDepth = length( WorldPos.xyz - cameraPosition.xyz );
	
#endif
	
	float shadowFactor = getShadowFactor( WorldPos.xyz, normal.xyz, linDepth );
	return shadowFactor;
}
