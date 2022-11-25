#include "globalConstants.inc"

cbuffer ModelConstants : register( b1 )
{
  float4x4 matWorld;
  float4x4 matLastFrameWorld;
  float4 instanceId; //yzw unused
  float4 tintColour;
};

#ifdef GS_CUBE_OUTPUT

cbuffer CubeMtxConstantsGS : register( b2 )
{
  float4x4 matCubeFaceProjView[ 6 ];
}

#endif

#if HAS_BONE_DATA

StructuredBuffer<float4x4> skinningMatrices : register( t2 );

#endif

#if INSTANCED_XFMS
StructuredBuffer<float4> instData : register( t3 );
#endif

StructuredBuffer<float4> damagePoint : register( t1 );

#if (HAS_TANGENT == 0)
  #undef NORMAL_MAP
#endif

#if DIFFUSE_MAP
  sampler DiffuseSampler : register(s0);
  Texture2D<float4> DiffuseTex : register(t0);
#endif

#if NORMAL_MAP
  Texture2D<float4> NormalTex : register(t1);
#endif

#if SPEC_MAP
  Texture2D<float4> SpecTex : register(t2);
#endif

#if MASK_MAP
  Texture2D<float4> MaskTex : register(t3);
#endif

#if CUBE_MAP
	TextureCube<float4> CubeTex : register(t4);
	#undef HEIGHT_MAP //currently no free sampler for heightmap, so can't have both cube and heightmaps. fred: fix this!
#endif

#if HEIGHT_MAP
	Texture2D<float4> HeightTex : register(t4);
#endif

#define EPSILON 0.001f

/*

Util funcs

*/

float3 projectOnPlane(float3 p, float3 planePoint, float3 planeNormal)
{
  return p - dot(p - planePoint, planeNormal) * planeNormal;
}

float getPackedSpecPower( float specPower )
{
  #define MAX_SPEC_POWER 200.f
  return saturate( specPower / MAX_SPEC_POWER ); 
}

/*

Vertex Shader

*/

struct VertexInput 
{
  float4 pos 		  : POSITION;
#if (DEBUG_DIFFUSE == 0)
  float3 normal 	: NORMAL;
  #if HAS_TANGENT
    float3 tangent  : TANGENT;
  #endif
#endif
#if HAS_UVS
  float2 uv 		  : TEXCOORD0;
#endif
#if HAS_BONE_DATA
	uint4 boneIndices  : BONES;
	float4 boneWeights : WEIGHTS;
#endif
};

struct VertexOutput
{
#if TESSELLATION || GEOM_SHADER
  float3 worldPos		: WORLDPOS;
  float3 lastFrameWorldPos: WORLDPOS1;
#else
  float4 hpos		: SV_Position;
  float4 hLastFramePos : TEXCOORD2;
  float4 hCurrentFramePos : TEXCOORD3;
#endif

#if (DEBUG_DIFFUSE == 0)
  float3 normal 	: TEXCOORD0;
#endif
#if (DIFFUSE_MAP || NORMAL_MAP)
  float2 uv 		: TEXCOORD1;
#endif
#if NORMAL_MAP
  float3 tangent : TEXCOORD4;
#endif
#if HEIGHT_MAP
  float3 viewDir : TEXCOORD5;
#endif
};

#if INSTANCED_XFMS
VertexOutput main_vs( in VertexInput v, in uint instId : SV_InstanceId )
#else
VertexOutput main_vs( in VertexInput v )
#endif
{
  VertexOutput o = (VertexOutput)0;
  
  float4 localPos = v.pos;
  
#if HAS_BONE_DATA
	float4x4 boneTransform = skinningMatrices[ v.boneIndices[ 0 ] ] * v.boneWeights[ 0 ];
    boneTransform += skinningMatrices[ v.boneIndices[ 1 ] ] * v.boneWeights[ 1 ];
    boneTransform += skinningMatrices[ v.boneIndices[ 2 ] ] * v.boneWeights[ 2 ];
    boneTransform += skinningMatrices[ v.boneIndices[ 3 ] ] * v.boneWeights[ 3 ];

	localPos = mul( boneTransform, localPos );
#endif
  
#if TESSELLATION || GEOM_SHADER
  o.worldPos = mul( matWorld, localPos ).xyz;
  o.lastFrameWorldPos = mul( matLastFrameWorld, localPos ).xyz;
#else
    #if INSTANCED_XFMS
      float4 p = float4( ( localPos.xyz * instData[ instId ].w ), 1.f );
      p.xyz += instData[ instId ].xyz;

      o.hpos = mul( matViewProj, p );
      o.hLastFramePos = o.hpos;
      o.hCurrentFramePos = o.hpos;
    #else
      float4x4 matWVP = mul( matViewProj, matWorld );
      o.hpos = mul( matWVP, localPos );

      float4x4 matLastFrameWVP = mul( matLastFrameViewProj, matLastFrameWorld );
      o.hLastFramePos = mul( matLastFrameWVP, localPos );

      o.hCurrentFramePos = o.hpos;
    #endif
#endif

#if (DEBUG_DIFFUSE == 0)
  #if HAS_BONE_DATA
    v.normal = mul( boneTransform, float4( v.normal, 0.f ) ).xyz;
	
	#if NORMAL_MAP
	  v.tangent = mul( boneTransform, float4( v.tangent, 0.f ) ).xyz;
	#endif
	
  #endif
  o.normal = normalize( mul( matWorld, float4( v.normal, 0.f ) ).xyz);

  #if NORMAL_MAP
    o.tangent = mul( matWorld, float4( v.tangent, 0.f ) ).xyz;
	
	#if HEIGHT_MAP
      float3 worldPos = mul( matWorld, localPos ).xyz;
	  o.viewDir = normalize( cameraPosition.xyz - worldPos.xyz );
	#endif
  #endif

  #if DIFFUSE_MAP
    o.uv = v.uv;
  #endif
#endif
  return o;
} 


#if TESSELLATION
/*

Hull Shader

*/

struct HSPerPatchOutput
{
  float edgeTessFactors[3] : SV_TessFactor;
  float insideTessFactor[1] : SV_InsideTessFactor;

  #if PNT_SMOOTHING
    // Geometry cubic generated control points
    float3 f3B210    : POSITION3;
    float3 f3B120    : POSITION4;
    float3 f3B021    : POSITION5;
    float3 f3B012    : POSITION6;
    float3 f3B102    : POSITION7;
    float3 f3B201    : POSITION8;
    float3 f3B111    : CENTER;
  #endif
    float isDestroyed : TEXCOORD2;
};

HSPerPatchOutput perPatchFunc( InputPatch<VertexOutput, 3> ip, uint patchID : SV_PrimitiveID )
{
  HSPerPatchOutput o = (HSPerPatchOutput)0;

  o.edgeTessFactors[0] = o.edgeTessFactors[1] = o.edgeTessFactors[2] = 5.f;
  o.insideTessFactor[0] = 1.f;

  #if PNT_SMOOTHING
    
    float3 f3B003 = ip[0].worldPos;
    float3 f3B030 = ip[1].worldPos;
    float3 f3B300 = ip[2].worldPos;
            
    float3 f3N002 = ip[0].normal;
    float3 f3N020 = ip[1].normal;
    float3 f3N200 = ip[2].normal;

    // Compute the cubic geometry control points
    // Edge control points
    o.f3B210 = ( ( 2.0f * f3B003 ) + f3B030 - ( dot( ( f3B030 - f3B003 ), f3N002 ) * f3N002 ) ) / 3.0f;
    o.f3B120 = ( ( 2.0f * f3B030 ) + f3B003 - ( dot( ( f3B003 - f3B030 ), f3N020 ) * f3N020 ) ) / 3.0f;
    o.f3B021 = ( ( 2.0f * f3B030 ) + f3B300 - ( dot( ( f3B300 - f3B030 ), f3N020 ) * f3N020 ) ) / 3.0f;
    o.f3B012 = ( ( 2.0f * f3B300 ) + f3B030 - ( dot( ( f3B030 - f3B300 ), f3N200 ) * f3N200 ) ) / 3.0f;
    o.f3B102 = ( ( 2.0f * f3B300 ) + f3B003 - ( dot( ( f3B003 - f3B300 ), f3N200 ) * f3N200 ) ) / 3.0f;
    o.f3B201 = ( ( 2.0f * f3B003 ) + f3B300 - ( dot( ( f3B300 - f3B003 ), f3N002 ) * f3N002 ) ) / 3.0f;
    // Center control point
    float3 f3E = ( o.f3B210 + o.f3B120 + o.f3B021 + o.f3B012 + o.f3B102 + o.f3B201 ) / 6.0f;
    float3 f3V = ( f3B003 + f3B030 + f3B300 ) / 3.0f;
    o.f3B111 = f3E + ( ( f3E - f3V ) / 2.0f );
  #endif

  o.isDestroyed = 0.f;

  if(distance(ip[0].worldPos, damagePoint[0].xyz) < 2.f)
  {
    o.edgeTessFactors[0] = o.edgeTessFactors[1] = o.edgeTessFactors[2] = 32.f;
    o.insideTessFactor[0] = 32.f;
    o.isDestroyed = 1.f;
  }
  else if(distance(ip[1].worldPos, damagePoint[0].xyz) < 2.f)
  {
    o.edgeTessFactors[0] = o.edgeTessFactors[1] = o.edgeTessFactors[2] = 32.f;
    o.insideTessFactor[0] = 32.f;
    o.isDestroyed = 1.f;
  }
  else if(distance(ip[2].worldPos, damagePoint[0].xyz) < 2.f)
  {
    o.edgeTessFactors[0] = o.edgeTessFactors[1] = o.edgeTessFactors[2] = 32.f;
    o.insideTessFactor[0] = 32.f;
    o.isDestroyed = 1.f;
  }

  return o;
}

struct HullOutput
{
  float3 worldPos		: WORLDPOS;
  float3 lastFrameWorldPos : WORLDPOS1;

  #if (DEBUG_DIFFUSE == 0)
    float3 normal 	: TEXCOORD0;
  #endif
  #if (DIFFUSE_MAP || NORMAL_MAP)
    float2 uv 		: TEXCOORD1;
  #endif
  #if NORMAL_MAP
    float3 tangent : TEXCOORD4;
  #endif
  #if HEIGHT_MAP
    float3 viewDir : TEXCOORD5;
  #endif
};


[domain("tri")]
[partitioning("pow2")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("perPatchFunc")]
HullOutput main_hs( InputPatch<VertexOutput, 3> p, uint i : SV_OutputControlPointID )
{
  HullOutput o = (HullOutput)0;

  o.worldPos = p[i].worldPos;
  o.lastFrameWorldPos = p[i].lastFrameWorldPos;

#if (DEBUG_DIFFUSE == 0)
  o.normal = p[i].normal;
#endif

#if NORMAL_MAP
  o.tangent = p[i].tangent;
#endif

#if HEIGHT_MAP
  o.viewDir = p[i].viewDir;
#endif

#if DIFFUSE_MAP
  o.uv = p[i].uv;
#endif

  return o;
}


/*

Domain Shader

*/

struct DomainOutput
{
#if GEOM_SHADER
  float3 worldPos	: WORLDPOS;
  float3 lastFrameWorldPos : WORLDPOS1;
#else
  float4 hpos		  : SV_Position;
  float4 hLastFramePos : WORLDPOS1;
  float4 hCurrentFramePos : WORLDPOS2;
#endif

#if (DEBUG_DIFFUSE == 0)
  float3 normal 	: TEXCOORD0;
#endif
#if (DIFFUSE_MAP || NORMAL_MAP)
  float2 uv 		: TEXCOORD1;
#endif
#if NORMAL_MAP
  float3 tangent : TEXCOORD4;
#endif
#if HEIGHT_MAP
  float3 viewDir : TEXCOORD5;
#endif
};

[domain("tri")]
DomainOutput main_ds( HSPerPatchOutput i, float3 barycentricCoords : SV_DomainLocation, const OutputPatch<HullOutput, 3> patch )
{
  DomainOutput o = (DomainOutput)0;

  float3 worldPos = barycentricCoords.x * patch[0].worldPos + 
                    barycentricCoords.y * patch[1].worldPos + 
                    barycentricCoords.z * patch[2].worldPos;

 float3 lastFrameWorldPos = barycentricCoords.x * patch[0].lastFrameWorldPos + 
                    barycentricCoords.y * patch[1].lastFrameWorldPos + 
                    barycentricCoords.z * patch[2].lastFrameWorldPos;

#if PHONG_SMOOTHING

  float3 projected_p0 = projectOnPlane(worldPos, patch[0].worldPos, normalize(patch[0].normal));
  float3 projected_p1 = projectOnPlane(worldPos, patch[1].worldPos, normalize(patch[1].normal));
  float3 projected_p2 = projectOnPlane(worldPos, patch[2].worldPos, normalize(patch[2].normal));

  worldPos = barycentricCoords.x * projected_p0 + 
                    barycentricCoords.y * projected_p1 + 
                    barycentricCoords.z * projected_p2;

#endif

#if PNT_SMOOTHING
  
  // The barycentric coordinates
  float fU = barycentricCoords.x;
  float fV = barycentricCoords.y;
  float fW = barycentricCoords.z;

  // Precompute squares and squares * 3 
  float fUU = fU * fU;
  float fVV = fV * fV;
  float fWW = fW * fW;
  float fUU3 = fUU * 3.0f;
  float fVV3 = fVV * 3.0f;
  float fWW3 = fWW * 3.0f;
    
  // Compute position from cubic control points and barycentric coords
  worldPos = patch[0].worldPos * fWW * fW +
             patch[1].worldPos * fUU * fU +
             patch[2].worldPos * fVV * fV +
             i.f3B210 * fWW3 * fU +
             i.f3B120 * fW * fUU3 +
             i.f3B201 * fWW3 * fV +
             i.f3B021 * fUU3 * fV +
             i.f3B102 * fW * fVV3 +
             i.f3B012 * fU * fVV3 +
             i.f3B111 * 6.0f * fW * fU * fV;

  float3 normal = barycentricCoords.z * patch[0].normal + 
                    barycentricCoords.x * patch[1].normal + 
                    barycentricCoords.y * patch[2].normal;

  o.normal = normalize(normal);

  #if NORMAL_MAP
    o.tangent = barycentricCoords.z * patch[0].tangent + 
                    barycentricCoords.x * patch[1].tangent + 
                    barycentricCoords.y * patch[2].tangent;
  #endif

  #if HEIGHT_MAP
    o.viewDir = barycentricCoords.z * patch[0].viewDir + 
                    barycentricCoords.x * patch[1].viewDir + 
                    barycentricCoords.y * patch[2].viewDir;
  #endif
  
  #if DIFFUSE_MAP
    o.uv = barycentricCoords.z * patch[0].uv + 
           barycentricCoords.x * patch[1].uv + 
           barycentricCoords.y * patch[2].uv;
  #endif

#else

  #if (DEBUG_DIFFUSE == 0)
    float3 normal = barycentricCoords.x * patch[0].normal + 
                    barycentricCoords.y * patch[1].normal + 
                    barycentricCoords.z * patch[2].normal;

    o.normal = normalize(normal);
  #endif

  #if NORMAL_MAP
    o.tangent = barycentricCoords.x * patch[0].tangent + 
                    barycentricCoords.y * patch[1].tangent + 
                    barycentricCoords.z * patch[2].tangent;
  #endif
  
  #if HEIGHT_MAP
    o.viewDir = barycentricCoords.x * patch[0].viewDir + 
                    barycentricCoords.y * patch[1].viewDir + 
                    barycentricCoords.z * patch[2].viewDir;
  #endif

  #if DIFFUSE_MAP
  o.uv = barycentricCoords.x * patch[0].uv + 
              barycentricCoords.y * patch[1].uv + 
              barycentricCoords.z * patch[2].uv;
  #endif

#endif

  const float radius = 1.f;
  if(i.isDestroyed)
  {
    float3 ImpactPtToWorldPos = worldPos - damagePoint[0].xyz;
    float lengthVector = length(ImpactPtToWorldPos);
    if(lengthVector < radius)
    {
      float3 scaledVector = radius / lengthVector * ImpactPtToWorldPos;

      float3 targetPt = damagePoint[0].xyz + damagePoint[1].xyz;
      
      //float f = dot(normalize(scaledVector), damagePoint[1].xyz);

      float3 newWorldPos = scaledVector + damagePoint[0].xyz;
      float3 wpToNwp = normalize(worldPos - newWorldPos);
      float3 wpToTp = normalize(worldPos - targetPt);

      float f = dot(wpToNwp, wpToTp) + 0.2f;

      //worldPos = newWorldPos;
      worldPos = worldPos - wpToTp * 0.6f;

      worldPos = (worldPos + newWorldPos) / 2.f;

      #if DIFFUSE_MAP
          o.uv = float2(0.f, 0.f);
      #endif

      #if (DEBUG_DIFFUSE == 0)
        o.normal = -wpToTp;
       // o.normal = float3(f, f, f);
      #endif
    }
  }

  #if GEOM_SHADER
    o.worldPos = worldPos;
    o.lastFrameWorldPos = lastFrameWorldPos;
  #else
    o.hpos = mul(matViewProj, float4(worldPos, 1.f)); 
    o.hLastFramePos = mul(matLastFrameViewProj, float4(lastFrameWorldPos, 1.f));
    o.hCurrentFramePos = mul(matViewProj, float4(worldPos, 1.f));
  #endif
    
  return o;
} 


#endif // TESSELLATION

#if GEOM_SHADER
struct GeomOutput
{
  float4 hpos		  : SV_Position;
  float4 hLastFramePos : WORLDPOS1;

#if (DEBUG_DIFFUSE == 0)
  float3 normal 	: TEXCOORD0;
#endif
#if (DIFFUSE_MAP || NORMAL_MAP)
  float2 uv 		: TEXCOORD1;
#endif
#if NORMAL_MAP
  float3 tangent : TEXCOORD4;
#endif
#if HEIGHT_MAP
  float3 viewDir : TEXCOORD5;
#endif
#ifdef GS_CUBE_OUTPUT
	uint rtSliceIndex : SV_RenderTargetArrayIndex;
#endif
};

#ifdef GS_CUBE_OUTPUT
[maxvertexcount(18)]
#else
[maxvertexcount(3)]
#endif
void main_gs(
          #if TESSELLATION
            triangle DomainOutput i[3],
          #else
            triangle VertexOutput i[3],
          #endif 
            inout TriangleStream<GeomOutput> os)
{
  GeomOutput o = (GeomOutput)0;

  //float3 triNormal = normalize(cross(i[1].worldPos - i[0].worldPos, i[2].worldPos - i[0].worldPos));

#ifdef GS_CUBE_OUTPUT
  for( int f = 0; f < 6; ++f )
  {
    o.rtSliceIndex = f;
#endif
		
	for(uint it = 0; it < 3; it++)
	{
		float3 worldPos = i[it].worldPos;
		float3 lastFrameWorldPos = i[it].lastFrameWorldPos;
    
		#if (DEBUG_DIFFUSE == 0)
			o.normal = i[it].normal;
		#endif

		#if NORMAL_MAP
			o.tangent = i[it].tangent;
		#endif
		
		#if HEIGHT_MAP
			o.viewDir = i[it].viewDir;
		#endif

		#if DIFFUSE_MAP
			o.uv = i[it].uv;
		#endif

		//worldPos += triNormal * 0.2f;
		#ifdef GS_CUBE_OUTPUT	
			o.hpos = mul( matCubeFaceProjView[ f ], float4( worldPos, 1.f ) );
			o.hLastFramePos = mul( matCubeFaceProjView[ f ], float4( lastFrameWorldPos , 1.f ) ); //currently no motion blur for cubemaps. Todo: make velocity ouput optional
		#else
			o.hpos = mul( matViewProj, float4( worldPos, 1.f ) );
			o.hLastFramePos = mul( matLastFrameViewProj, float4( lastFrameWorldPos , 1.f ) );
		#endif
		os.Append(o);
		//os.RestartStrip();
	}
	
#ifdef GS_CUBE_OUTPUT	
    os.RestartStrip();
  }
#endif

}

#endif //GEOM_SHADER


/*

Pixel Shader

*/

struct PixelOutput
{
#if DEBUG_DIFFUSE
  float4 diffuseRT 	: SV_Target0;
#else
  float4 normalRT 	: SV_Target0;
  float4 diffuseRT 	: SV_Target1;
  #if OUTPUT_VELOCITY
    float2 velocityRT : SV_Target2;
  #endif
  #if MESH_ID
	float meshIdRT : SV_Target3;
  #endif
#endif
};

PixelOutput main_ps( 
                      #if GEOM_SHADER
                          in GeomOutput f 
                      #else
                        #if TESSELLATION
                          in DomainOutput f 
                        #else
                          in VertexOutput f 
                        #endif
                      #endif
                      )
{	
  PixelOutput o;

  #if (DIFFUSE_MAP || NORMAL_MAP)
    f.uv.y = 1 - f.uv.y;
  #endif

  o.diffuseRT = tintColour;

  #if DIFFUSE_MAP	
    o.diffuseRT *= DiffuseTex.Sample( DiffuseSampler, f.uv );
  #endif

#if !DEBUG_DIFFUSE

  #if NORMAL_MAP
    float3 tangent = normalize( f.tangent.xyz );
	  float3 vertNormal = normalize( f.normal.xyz );
    float3 binormal = normalize( cross( vertNormal, tangent ) );
	
	#if HEIGHT_MAP
		float3 tangentSpaceViewDir = float3( dot( tangent, f.viewDir.xyz ), dot( binormal, f.viewDir.xyz ), dot( vertNormal, f.viewDir.xyz ) );
		float height = HeightTex.Sample( DiffuseSampler, f.uv ).x;
		f.uv += 1.0f *( height * 0.04f - 0.02f ) * tangentSpaceViewDir.xy;
	#endif
	
    float3 normal = NormalTex.Sample( DiffuseSampler, f.uv ).xyz * 2.f - 1.f;
    normal = normalize( normal.x * tangent + normal.y * binormal + normal.z * vertNormal );
  #else
    float3 normal = normalize(f.normal);
  #endif

  o.normalRT = float4( normal * 0.5f + 0.5f, getPackedSpecPower( 16.f ) );

  #if DEBUG_UVS && (DIFFUSE_MAP || NORMAL_MAP)
    o.diffuseRT = float4( f.uv.xy, 0.f, 1.f );
  #endif

  #if SPEC_MAP
    float specCoeff = SpecTex.Sample( DiffuseSampler, f.uv ).r;
    o.diffuseRT.w = specCoeff;
  #endif

  #if MASK_MAP
    float mask = MaskTex.Sample( DiffuseSampler, f.uv ).r;
    clip( mask - EPSILON );
  #endif
  
  #if CUBE_MAP && DIFFUSE_MAP
	  float4 reflection = CubeTex.Sample( DiffuseSampler, normal );
	  o.diffuseRT = reflection;
  #endif

  #if OUTPUT_VELOCITY
    if( f.hLastFramePos.w <= EPSILON )
    {
      f.hLastFramePos = lerp( f.hCurrentFramePos, f.hLastFramePos, (f.hCurrentFramePos.w - EPSILON) / (f.hCurrentFramePos.w - f.hLastFramePos.w ) );
    }

    float4 currentPos = f.hCurrentFramePos / f.hCurrentFramePos.w;
    float4 lastPos = f.hLastFramePos / f.hLastFramePos.w; 
    float2 velocity = ( lastPos.xy - currentPos.xy ) * 0.5f;
    o.velocityRT = float2( -velocity.x, velocity.y );
  #endif
  
  #if MESH_ID
	  o.meshIdRT = instanceId.x;
  #endif

#endif //!DEBUG_DIFFUSE

  return o;

}
