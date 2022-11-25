cbuffer GlobalConstants : register( b0 )
{
  float4x4 matView;
  float4x4 matProj;
  float4x4 matViewProj;
  float4x4 matInvViewProj;
  float4x4 matInvProj;
  float4x4 matLastFrameViewProj;
  float4x4 matInvProjViewRescaledPixelPos;
  float4x4 matInvProjViewRescaledUVs;
  float3   cameraPosition;
  float    timeFromStart;
  float4   backBufferDimensions;
  float4   projectionPlanes;
  float4   viewSpaceExtents;
};


