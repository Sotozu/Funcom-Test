#ifdef STEREO
cbuffer StereoConstants : register( b4 )
{
  float4x4 matViewProjStereo[ 2 ];
  float4x4 matViewStereo[ 2 ];
  float4x4 matProjStereo[ 2 ];
  float4x4 matInvViewStereo[ 2 ];
  float4x4 matInvProjStereo[ 2 ];
  float4x4 matInvProjViewStereo[ 2 ];
  float4 cameraPositionStereo[ 2 ];
};

uint getStereoConstantIndexFromUV( float2 uv )
{
  return ( uv.x > 0.5f );
}

uint getStereoConstantIndexFromTexelPos( float2 texelPos, float surfaceWidth )
{
  return ( texelPos.x > ( surfaceWidth * 0.5f ) );
}

uint getStereoConstantIndexFromClipPos( float2 cp )
{
  return ( cp.x > 0.f );
}

float2 getStereoRescaledUVs( float2 uv, uint eyeIndex )
{
  float bias[ 2 ] = { 0.f, 1.f };
  return float2( ( uv.x * 2.f ) - bias[ eyeIndex ], uv.y );
}

#endif
