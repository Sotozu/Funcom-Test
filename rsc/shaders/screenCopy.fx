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

#if QUAD_OVERRIDE || COL_RESCALE || FLAT_COL
  cbuffer quadConstants : register( b1 )
  {
#if QUAD_OVERRIDE
    float4 vertexPos[ 4 ];
#endif
#if COL_RESCALE
    float4 rescaleFactor;
    float4 rescaleOffset;
#endif
#if FLAT_COL
    float4 flatColour;
#endif
  }
#endif

sampler DiffuseSampler;
Texture2D<float4> DiffuseTex;

VertexOutput main_vs( in VertexInput v, uint vertexId : SV_VertexID )
{
	VertexOutput o;
	
	#if QUAD_OVERRIDE
	  o.hpos = vertexPos[ vertexId ];
	#else
	  o.hpos = v.pos;
	#endif
	o.uv = v.uv;

	return o;
} 

float4 main_ps( in VertexOutput f ) : SV_Target0
{	
#if FLAT_COL
  float4 color = flatColour;
#else
	float4 color = DiffuseTex.Sample( DiffuseSampler, f.uv ); 
#endif

#if COL_RESCALE
  color = color * rescaleFactor + rescaleOffset;
#endif

	return color;
}
