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


cbuffer quadConstants : register( b1 )
{
  float4 slice_res;
}


sampler linearSampler;
Texture3D< float4 > volTex;

VertexOutput main_vs( in VertexInput v, uint vertexId : SV_VertexID )
{
	VertexOutput o;
	
	o.hpos = v.pos;
	o.uv = v.uv;

	return o;
} 

float4 main_ps( in VertexOutput f ) : SV_Target0
{	
	float depthUV = saturate( slice_res.x / slice_res.w );
	float4 color = volTex.SampleLevel( linearSampler, float3( f.uv.xy, depthUV ), 0 ); 
	//return float4( depthUV.xxx, 1.f );
	return color;
}
