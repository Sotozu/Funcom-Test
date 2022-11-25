
/*
  Depth buffer related utility functions
*/

float getLinearDepth( float depth )
{
  return ( 1.f / ( projectionPlanes.z * depth + projectionPlanes.w ) );
}

float getCameraDepthFromLinear( float linDepth )
{
  return linDepth * projectionPlanes.y;
}

float getCameraDepth( float depth )
{
  return ( ( projectionPlanes.x * projectionPlanes.y ) / ( projectionPlanes.y - depth * ( projectionPlanes.y - projectionPlanes.x ) ) );
}

float3 getVSPosFromSS( float2 uv, float linDepth )
{
  float3 vsPos = float3( lerp( -viewSpaceExtents.x, viewSpaceExtents.x, uv.x ), lerp( -viewSpaceExtents.y, viewSpaceExtents.y, uv.y ), viewSpaceExtents.z );
  return vsPos * linDepth;
}


/*
  Noise related utility functions
*/

float2x2 getRandomRotMatrix( Texture2D noiseMap, SamplerState smp, float2 uv )
{
	float2 rot = normalize( 2.f * noiseMap.SampleLevel( smp, uv, 0 ).xy - 1.f );

	// | cos(a)		-sin(a) |
	// | sin(a)		 cos(a) |
	float2x2 rotMat = { rot.x, -rot.y, rot.y, rot.x };
	return rotMat;
}
