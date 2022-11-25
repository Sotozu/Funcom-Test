#include "sas_pch.h"
#include "sas_device_inputlayoutcache.h"
#include "sas_device_common.h"
#include "render/sas_vertexformat.h"
#include <D3DCompiler.h>

ID3D10Blob* GetCodeSignature( const sasVertexFormat& fmt );


sasDeviceInputLayoutCache::sasDeviceInputLayoutCache( ID3D11Device* device )
  : _device( device )
{

}

sasDeviceInputLayoutCache::~sasDeviceInputLayoutCache()
{
  for( auto i=_inputLayouts.begin(); i!=_inputLayouts.end(); i++ )
  {
    DX_RELEASE( (*i).second );
  }
}

ID3D11InputLayout* sasDeviceInputLayoutCache::createInputLayout( const sasVertexFormat& fmt )
{
  size_t nStreams = 0;
  D3D11_INPUT_ELEMENT_DESC descs[17];

  if( fmt.position == sasVertexFormat::POSITION_3F ) 
  {
    descs[nStreams].SemanticName = "POSITION";
    descs[nStreams].SemanticIndex = 0;
    descs[nStreams].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    descs[nStreams].InputSlot = 0;
    descs[nStreams].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    descs[nStreams].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    descs[nStreams].InstanceDataStepRate = 0;
    ++nStreams;
  }
  if( fmt.normal == sasVertexFormat::NORMAL_3F ) 
  {
    descs[nStreams].SemanticName = "NORMAL";
    descs[nStreams].SemanticIndex = 0;
    descs[nStreams].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    descs[nStreams].InputSlot = 0;
    descs[nStreams].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    descs[nStreams].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    descs[nStreams].InstanceDataStepRate = 0;
    ++nStreams;
  }
  if( fmt.normal == sasVertexFormat::TANGENT_3F ) 
  {
    descs[nStreams].SemanticName = "TANGENT";
    descs[nStreams].SemanticIndex = 0;
    descs[nStreams].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    descs[nStreams].InputSlot = 0;
    descs[nStreams].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    descs[nStreams].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    descs[nStreams].InstanceDataStepRate = 0;
    ++nStreams;
  }
  if( fmt.uv0 == sasVertexFormat::UV_2F ) 
  {
    descs[nStreams].SemanticName = "TEXCOORD";
    descs[nStreams].SemanticIndex = 0;
    descs[nStreams].Format = DXGI_FORMAT_R32G32_FLOAT;
    descs[nStreams].InputSlot = 0;
    descs[nStreams].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    descs[nStreams].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    descs[nStreams].InstanceDataStepRate = 0;
    ++nStreams;
  }
  if( fmt.boneIndices == sasVertexFormat::BONEINDICES ) 
  {
    descs[nStreams].SemanticName = "BONES";
    descs[nStreams].SemanticIndex = 0;
    descs[nStreams].Format = DXGI_FORMAT_R32G32B32A32_UINT;
    descs[nStreams].InputSlot = 0;
    descs[nStreams].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    descs[nStreams].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    descs[nStreams].InstanceDataStepRate = 0;
    ++nStreams;
  }
  if( fmt.boneWeights == sasVertexFormat::BONEWEIGHTS ) 
  {
    descs[nStreams].SemanticName = "WEIGHTS";
    descs[nStreams].SemanticIndex = 0;
    descs[nStreams].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    descs[nStreams].InputSlot = 0;
    descs[nStreams].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    descs[nStreams].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    descs[nStreams].InstanceDataStepRate = 0;
    ++nStreams;
  }

  ID3D10Blob* shCode = GetCodeSignature( fmt );
  ID3D11InputLayout* layout = 0;
  HRESULT hr;
  DX( _device->CreateInputLayout( descs, nStreams, shCode->GetBufferPointer(), shCode->GetBufferSize(), &layout ) );
  DX_RELEASE( shCode );

  return layout;
}

ID3D11InputLayout* sasDeviceInputLayoutCache::getInputLayout( const sasVertexFormat& fmt )
{
  InputLayouts::iterator it = _inputLayouts.find( fmt );  
  
  if( (it != _inputLayouts.end()) && (it->first.flags == fmt.flags) )
    return (*it).second;

  ID3D11InputLayout* inputLayout = createInputLayout( fmt );
  sasASSERT( inputLayout );

  _inputLayouts[ fmt ] = inputLayout;

  return inputLayout;
}

  

ID3D10Blob* GetCodeSignature( const sasVertexFormat& fmt )
{
  std::string code = "struct Input {\n"; 

  if( fmt.position  == sasVertexFormat::POSITION_3F ) code += " float4 pos : POSITION;\n";
  if( fmt.normal    == sasVertexFormat::NORMAL_3F )   code += " float4 normal : NORMAL;\n";
  if( fmt.tangent   == sasVertexFormat::TANGENT_3F )   code += " float4 tangent : TANGENT;\n";
  if( fmt.uv0       == sasVertexFormat::UV_2F )       code += " float2 uv0    : TEXCOORD0;\n";
  if( fmt.boneIndices == sasVertexFormat::BONEINDICES ) code += " uint4 boneIndices  : BONES;\n";
  if( fmt.boneWeights == sasVertexFormat::BONEWEIGHTS ) code += " float4 boneWeights : WEIGHTS;\n";


  code += "};\n"
    "float4 main( in Input i ) : POSITION { return i.pos; }\n";

  HRESULT hr;
  ID3D10Blob* shCode = 0; 
  ID3D10Blob* errMsg = 0;
  
#ifdef USE_D3DX
  DX( D3DX11CompileFromMemory(
    code.c_str(),
    code.size(),
    0, 0, 0,
    "main",
    "vs_5_0",
    D3D10_SHADER_ENABLE_STRICTNESS, 
    0, 0,
    &shCode, 
    &errMsg, 
    0
    ) );
#else
  DX( D3DCompile( code.c_str() ,  code.size(), nullptr, nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,0, &shCode, &errMsg ) );
#endif

  if( errMsg )
  {
    OutputDebugString( (LPCSTR)errMsg->GetBufferPointer() );
  }

  DX_RELEASE( errMsg );

  return shCode;
}