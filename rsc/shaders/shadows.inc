cbuffer GlobalConstants : register( b3 )
{
  float4x4 matShadowProjView[ 3 ];
  float4 cascadeDistances;
  float4 lightDirGodrayIntensity;
  float4 invSMSize_invGMSize;
};

Texture2D< float > shadowMapC0 	: register( t5 );
Texture2D< float > shadowMapC1 	: register( t6 );
Texture2D< float > shadowMapC2 	: register( t7 );
SamplerComparisonState shadowSampler 	: register( s7 );

cbuffer cb4 : register( b5 )
{
  float4x4  spotShadowProjView[ 4 ];
  float4    spotInvSMSize[ 4 ];
};

Texture2D< float > spotSM0 	: register( t8 );
Texture2D< float > spotSM1 	: register( t9 );
Texture2D< float > spotSM2 	: register( t10 );
Texture2D< float > spotSM3 	: register( t11 );

int getCascadeIndex( float linDepth )
{
	if( linDepth < cascadeDistances.x )
		return 0;
	if( linDepth < cascadeDistances.y )
		return 1;

	return 2;
}

float sampleKernel( Texture2D< float > sm, SamplerComparisonState smSampler, float2 uv, float refDepth, float2 invSmSize, float kernelSize )
{
  return 1.f;
}

float getShadowFactor( float3 wsPosition, float3 wsNormal, float linDepth )
{
	return 1.f;
}


float getRayMarchedShadowAccumulation( float3 wsStartPosition, float3 wsEndPosition, uint numSteps )
{
	return 0.f;
}


float2 getShadowFactorDebugUVs( float3 wsPosition )
{
	float4 lsPos = mul( matShadowProjView[ 0 ], float4( wsPosition.xyz, 1.f ) );
	lsPos /= lsPos.w;
	
	float2 shadowCoords = float2( lsPos.x * 0.5f + 0.5f, 1.f - ( lsPos.y * 0.5f + 0.5f ) );
	
	return shadowCoords;
}


float getShadowFactorForSpot( uint index, float3 wsPosition, float3 wsNormal, float3 wsPointToLight )
{
  return 1.f;
}

