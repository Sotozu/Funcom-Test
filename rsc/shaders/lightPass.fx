#include "globalConstants.inc"
#include "shadows.inc"
#include "stereo.inc"

RWTexture2D<float4> lightBuffer : register( u0 );
RWTexture2D<float4> specBuffer : register( u1 );

Texture2D normalMap : register( t2 );
Texture2D depthMap : register( t1 );
sampler SSAOSampler : register(s3);
Texture2D< float4 > SSAOMap : register(t3);
Texture2D screenSpaceShadowMap : register( t4 );


cbuffer cb1 : register( b1 )
{
  float4 sunDir;
  float4 sunCol;
  int numPointLights;
  int numSpotLights;
};



struct PointLightData
{
  float4 pos;
  float4 col_radius;
};

StructuredBuffer<PointLightData> PointLightDataArray : register( t0 );

struct SpotLightData
{
  float4 pos_innerAngle;
  float4 col_radius;
  float4 dir_outerAngle;
  float  smIndex;
  float3 padding;
};

StructuredBuffer<SpotLightData> SpotLightDataArray : register( t5 );

RWStructuredBuffer<float4> damageVolumeOutput : register( u2 );
cbuffer cb2 : register( b2 )
{
  float4 damagePoint;
}



float unpackSpecPower( float packedSpecPower )
{
  return packedSpecPower * 200.f;
}

float2 ComputePointLightContribution( float3 WorldPos, float4 Normal_SP, float3 CamPos, PointLightData lightData )
{
  float3 cPointToLight = lightData.pos.xyz - WorldPos.xyz;
  float distToLight = length(cPointToLight);
  float3 L = normalize(cPointToLight);

  float attenuation = pow( max( 0.f, 1.f - ( distToLight / lightData.col_radius.w ) ), 2.f );

  float vIntensity = saturate(dot(Normal_SP.xyz, L)) * attenuation;

  float3 V = normalize(CamPos.xyz - WorldPos.xyz);
  float3 H = normalize(V + L);
  float vGloss = pow(saturate(dot(H, Normal_SP.xyz)), Normal_SP.w) * attenuation;

  return float2(vIntensity, vGloss);
}

float2 ComputeSpotLightContribution( float3 WorldPos, float4 Normal_SP, float3 CamPos, SpotLightData lightData )
{
  float3 cPointToLight = lightData.pos_innerAngle.xyz - WorldPos.xyz;
  float distToLight = length( cPointToLight );
  float3 L = normalize( cPointToLight );

  float attenuation = pow( max( 0.f, 1.f - ( distToLight / lightData.col_radius.w ) ), 2.f );

  float vIntensity = saturate( dot( Normal_SP.xyz, L ) ) * attenuation;
  
  float spotIntensity = dot( -L.xyz, lightData.dir_outerAngle.xyz );
  spotIntensity = smoothstep( lightData.dir_outerAngle.w, lightData.pos_innerAngle.w, spotIntensity );

  float shadowFactor = 1.f;
  [branch]
  if( ( lightData.smIndex >= 0 ) && ( spotIntensity > 0.f ) )
  {
    shadowFactor = getShadowFactorForSpot( lightData.smIndex, WorldPos.xyz, Normal_SP.xyz, L.xyz );
  }

  float3 V = normalize(CamPos.xyz - WorldPos.xyz);
  float3 H = normalize(V + L);
  float vGloss = pow(saturate(dot(H, Normal_SP.xyz)), Normal_SP.w) * attenuation;

  return float2(vIntensity, vGloss) * spotIntensity * shadowFactor;
}




float2 ComputeSunLightContribution( float depth, float3 WorldPos, float4 Normal_SP, float3 CamPos )
{
  float2 r = float2(0.f, 0.f);

  if(depth < 1.f)
  {
    //Directional diffuse
    r.x = saturate( dot( Normal_SP.xyz, -sunDir.xyz ) );

    //Directional specular
    float3 V = normalize( CamPos.xyz - WorldPos.xyz );
    float3 H = normalize( -sunDir.xyz + V );
    r.y = pow( saturate( dot( Normal_SP.xyz, H ) ), Normal_SP.w );  
	
	//fresnel
	float base = 1.f - dot( V, H );
	float exp = pow( base, 5.f );
	float fresnel = exp + 0.5f * ( 1.f - exp );
	
	//normalization
	#define EIGHT_PI 25.13274f
	float normalizationCoeff = ( Normal_SP.w + 2 ) / EIGHT_PI;
	
	r.y *= normalizationCoeff * fresnel;
  }

  return r;
}

#define OFFSET_TO_ORIGIN 10000.f

[numthreads (16, 16, 1)]
void main_cs( uint3 threadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex, uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID )
{


  //Data prep
#ifdef STEREO_RIGHT_EYE
  uint2 rescaledCoords = threadID.xy + ( backBufferDimensions.xy * float2( 0.5f, 0.f ) );
  float depth = depthMap.Load( int3( rescaledCoords.xy, 0 ) ).r;
  float4 normal = normalMap.Load( int3( rescaledCoords.xy, 0 ) );
#else
  float depth = depthMap.Load( int3( threadID.xy, 0 ) ).r;
  float4 normal = normalMap.Load( int3( threadID.xy, 0 ) );
#endif
  normal.xyz = normalize(normal.xyz * 2.f - 1.f);
  normal.w = unpackSpecPower( normal.w );
  
#ifdef STEREO_LEFT_EYE
  float2 uv = ( threadID.xy * float2( 2.f, 1.f ) ) / backBufferDimensions.xy;
  float4 HCoords = float4( 2.f * uv.x - 1.f, 1.f - 2.f * uv.y, depth, 1.f );
  float4 WorldPos = mul( matInvProjViewStereo[ 0 ], HCoords );
  WorldPos = WorldPos / WorldPos.w;
#elif STEREO_RIGHT_EYE
  float2 uv = ( threadID.xy * float2( 2.f, 1.f ) ) / backBufferDimensions.xy;
  float4 HCoords = float4( 2.f * uv.x - 1.f, 1.f - 2.f * uv.y, depth, 1.f );
  float4 WorldPos = mul( matInvProjViewStereo[ 1 ], HCoords );
  WorldPos = WorldPos / WorldPos.w;
#else  
  float4 HCoords = float4( 2.f * threadID.x / backBufferDimensions.x - 1.f, 1.f - 2.f * threadID.y / backBufferDimensions.y, depth, 1.f );
  float4 WorldPos = mul( matInvViewProj, HCoords );
  WorldPos = WorldPos / WorldPos.w;
#endif

  //Sunlight
#ifdef STEREO_RIGHT_EYE
  float shadowFactor = screenSpaceShadowMap.Load( int3( rescaledCoords.xy, 0 ) ).r;
#else
  float shadowFactor = screenSpaceShadowMap.Load( int3( threadID.xy, 0 ) ).r;
#endif
  float2 sunIntensity = ComputeSunLightContribution(depth, WorldPos.xyz, normal, cameraPosition.xyz) * shadowFactor.xx;
  float3 sunLight = sunIntensity.x * sunCol.xyz;
  float3 sunSpec = sunIntensity.y * sunCol.xyz;
  
  //temp ambient term
  float ambientCoeff = dot( normal.xyz, float3( 0.f, 1.f, 0.f ) ) * 0.5f + 0.5f;
  float3 ambient = lerp( float3(0.1f, 0.1f, 0.05f), float3( 0.3f, 0.25f, 0.15f ), ambientCoeff );
  float2 offsets = float2(2.f, 2.f) * backBufferDimensions.zw;
#ifdef STEREO_RIGHT_EYE
  float2 uv0 = rescaledCoords.xy * backBufferDimensions.zw;
#else
  float2 uv0 = threadID.xy * backBufferDimensions.zw;
#endif
  float ssao = SSAOMap.SampleLevel( SSAOSampler, uv0, 0 ).x;
  ambient *= ssao;
  

  float3 lightAccumulation = 0.f;
  float3 specAccumulation = 0.f;

lightAccumulation *= ssao * 0.5f + 0.5f;
lightAccumulation += sunLight.xyz + ambient.xyz;
specAccumulation += sunSpec.xyz;

#ifdef STEREO_RIGHT_EYE
  lightBuffer[ rescaledCoords.xy ] = float4( lightAccumulation, 1.f );
  specBuffer[ rescaledCoords.xy ] = float4( specAccumulation, 1.f );
#else
  lightBuffer[ threadID.xy ] = float4( lightAccumulation, 1.f );
  specBuffer[ threadID.xy ] = float4( specAccumulation, 1.f );
#endif
}
