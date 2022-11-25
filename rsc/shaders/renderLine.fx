#include "globalConstants.inc"

cbuffer LineDataCb : register( b1 )
{
  float4 vertexPositions[ 2 ];
  float4 col;
};

struct VertexOutput
{
  float4 pos : SV_Position;
  float4 col : COLOR;
};

VertexOutput main_vs( uint vertexID : SV_VertexID )
{
  VertexOutput o;
  
  o.pos = mul( matViewProj, float4( vertexPositions[ vertexID ].xyz, 1.f ) );
  o.col = col;
  
  return o;
} 

float4 main_ps( in VertexOutput f ) : SV_Target0
{	
  return f.col;
}
