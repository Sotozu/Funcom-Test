#include "globalConstants.inc"

struct VertexInput 
{
  float4 pos 		  : POSITION;
};

struct VertexOutput
{
  float4 worldPos : WORLDPOS;
};

VertexOutput main_vs( in VertexInput v, uint vertexID : SV_VertexID )
{
  VertexOutput o;
  
  o.worldPos = v.pos;
  
  return o;
} 

struct GeomOutput
{
  float4 hpos : SV_Position;
  float3 col : TEXCOORD0;
};


[maxvertexcount(3)]
void main_gs( triangle VertexOutput i[3], inout TriangleStream<GeomOutput> os )
{
  GeomOutput o = (GeomOutput)0;

  float3 triNormal = normalize(cross(i[1].worldPos - i[0].worldPos, i[2].worldPos - i[0].worldPos));

  for(uint it = 0; it < 3; it++)
  {
    float3 worldPos = i[it].worldPos;
    
    o.hpos = mul( matViewProj, float4(worldPos, 1.f) );
	o.col = triNormal * 0.5f + 0.5f;
    os.Append(o);
  }
}


float4 main_ps( in GeomOutput f ) : SV_Target0
{	
  return float4(f.col.xyz, 0.3f);
}
