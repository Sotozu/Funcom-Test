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

#ifdef vs_5_0
cbuffer ovrConstantsVS : register( b1 )
{
  float4x4 uvXform;
}
#endif

#ifdef ps_5_0
cbuffer ovrConstantsPS : register( b1 )
{
  float4 lensCenter_ScreenCenter;
  float4 scale_ScaleIn;
  float4 warpParams;
}
#endif

sampler linSampler;
Texture2D<float4> sceneTex;

#ifdef vs_5_0
VertexOutput main_vs( in VertexInput v, uint vertexId : SV_VertexID )
{
	VertexOutput o;

	o.hpos = v.pos;
	o.uv = mul( uvXform, float4( v.uv.xy, 0.f, 1.f ) ).xy;

	return o;
} 
#endif

#ifdef ps_5_0
float2 HmdWarp( float2 in01 )
{
  float2 theta = ( in01 - lensCenter_ScreenCenter.xy ) * scale_ScaleIn.zw; // Scales to [-1, 1]
  float  rSq = theta.x * theta.x + theta.y * theta.y;
  float2 theta1 = theta * ( warpParams.x + warpParams.y * rSq + 
                   warpParams.z * rSq * rSq + warpParams.w * rSq * rSq * rSq );
  return lensCenter_ScreenCenter.xy + scale_ScaleIn.xy * theta1;
}


float4 main_ps( in VertexOutput f ) : SV_Target0
{	
  float2 tc = HmdWarp( f.uv );
  if( any( clamp( tc, lensCenter_ScreenCenter.zw - float2( 0.25f, 0.5f ), lensCenter_ScreenCenter.zw + float2( 0.25f, 0.5f ) ) - tc ) )
    return 0;
	
  return sceneTex.Sample( linSampler, tc );
}
#endif
