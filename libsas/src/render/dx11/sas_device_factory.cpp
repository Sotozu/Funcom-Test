#include "sas_pch.h"
#include "sas_device_factory.h"
#include "sas_device_buffer.h"
#include "sas_device_common.h"
#include "sas_device_shader.h"
#include "sas_device_texture.h"
#include "sas_device_rendertarget.h"
#include "sas_device_depthbuffer.h"
#include "sas_device_gputimer.h"
#include "render/sas_indexbuffer.h"
#include "render/sas_vertexbuffer.h"
#include "render/sas_constantbuffer.h"
#include "render/sas_texture.h"
#include "render/sas_rendertarget.h"
#include "render/sas_depthbuffer.h"
#include "render/sas_structuredbuffer.h"
#include "render/sas_indirectargsbuffer.h"
#include "sas_device_typeconversion.h"
#include "render/sas_gputimer.h"
#include <sstream>

sasDeviceFactory::sasDeviceFactory( ID3D11Device* device )
  : _device( device )
{
}

sasDeviceFactory::~sasDeviceFactory()
{
}

sasDeviceObject* sasDeviceFactory::createIndexBuffer( const sasIndexBuffer& ib )
{
  sasDeviceBuffer* result = 0;  
  ID3D11Buffer* buffer = 0;
  D3D11_BUFFER_DESC desc = {0};
  D3D11_SUBRESOURCE_DATA data = {0};
  HRESULT hr;

  desc.ByteWidth = ib.size();
  desc.Usage = D3D11_USAGE_IMMUTABLE;
  desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  desc.CPUAccessFlags = 0;

  data.pSysMem = ib.buffer();

  DX( _device->CreateBuffer( &desc, &data, &buffer ) );
  if SUCCEEDED(hr)
  {
    result = sasNew sasDeviceBuffer( buffer, NULL, NULL );
  }  
  return result;
}

void sasDeviceFactory::releaseIndexBuffer( sasDeviceObject* obj )
{
  sasDelete (sasDeviceBuffer*)obj;
}

sasDeviceObject* sasDeviceFactory::createVertexBuffer( const sasVertexBuffer& vb )
{
  sasDeviceBuffer* result = 0;  
  ID3D11Buffer* buffer = 0;
  D3D11_BUFFER_DESC desc = {0};
  D3D11_SUBRESOURCE_DATA data = {0};
  HRESULT hr;

  desc.ByteWidth = vb.size();
  desc.Usage = D3D11_USAGE_IMMUTABLE;
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  desc.CPUAccessFlags = 0;

  data.pSysMem = vb.buffer();

  DX( _device->CreateBuffer( &desc, &data, &buffer ) );
  if SUCCEEDED(hr)
  {
    result = sasNew( sasDeviceBuffer )( buffer, NULL, NULL );
  }  
  return result;
}

void sasDeviceFactory::releaseVertexBuffer( sasDeviceObject* obj )
{
  sasDelete (sasDeviceBuffer*)obj;
}

sasDeviceObject* sasDeviceFactory::createShader( const char* file, const char* entryFunc, sasShaderType::Enum shaderType, sasShaderMacro* macros, size_t nMacros )
{
  sasDeviceObject* result = 0;
  const char* profile = 0;
  ID3D10Blob* shaderCode = 0;
  ID3D10Blob* errorMsg = 0;
  HRESULT hr;

  // profile
  switch( shaderType )
  {
  case sasShaderType::Vertex:   profile = "vs_5_0"; break;
  case sasShaderType::Hull:     profile = "hs_5_0"; break;
  case sasShaderType::Domain:   profile = "ds_5_0"; break;
  case sasShaderType::Geometry: profile = "gs_5_0"; break;
  case sasShaderType::Pixel:    profile = "ps_5_0"; break;
  case sasShaderType::Compute:  profile = "cs_5_0"; break;
  default: sasASSERT(0);
  }   

  // flags
  D3D_SHADER_MACRO* dxmacros = new D3D_SHADER_MACRO[ nMacros + 2 ]; 
  for( size_t i=0; i<nMacros; i++ )
  {
    dxmacros[i].Name = macros[i].define;
    dxmacros[i].Definition = macros[i].value;    
  }
  dxmacros[nMacros].Name = profile;
  dxmacros[nMacros].Definition = 0;
  dxmacros[nMacros + 1].Name = dxmacros[nMacros + 1].Definition = 0;

  // build
#ifdef USE_D3DX
  DX( 
    D3DX11CompileFromFile( 
    file, 
    dxmacros,
    0, 
    entryFunc, 
    profile,
    D3D10_SHADER_ENABLE_STRICTNESS|D3D10_SHADER_IEEE_STRICTNESS|D3D10_SHADER_OPTIMIZATION_LEVEL1|D3D10_SHADER_PACK_MATRIX_ROW_MAJOR,
    0,
    0,
    &shaderCode,
    &errorMsg,
    0 ) );  
#else
  std::wstringstream shData;
  shData << file;
  DX( 
    D3DCompileFromFile(
    shData.str().c_str(), 
    dxmacros,
    D3D_COMPILE_STANDARD_FILE_INCLUDE, 
    entryFunc, 
    profile,
    D3D10_SHADER_ENABLE_STRICTNESS|D3D10_SHADER_IEEE_STRICTNESS|D3D10_SHADER_OPTIMIZATION_LEVEL1|D3D10_SHADER_PACK_MATRIX_ROW_MAJOR,
    0,
    &shaderCode,
    &errorMsg ) );
#endif

  if( errorMsg )
  {
    OutputDebugString( (LPCTSTR)errorMsg->GetBufferPointer() );
  }
  DX_RELEASE( errorMsg );

  if SUCCEEDED( hr )
  {
    union 
    {
      void*                 d3dPtr;
      ID3D11VertexShader*   d3dVertexShader;
      ID3D11HullShader*     d3dHullShader;
      ID3D11DomainShader*   d3dDomainShader;
      ID3D11GeometryShader* d3dGeometryShader;
      ID3D11PixelShader*    d3dPixelShader;    
      ID3D11ComputeShader*  d3dComputeShader;
    } shader;

    switch( shaderType )
    {
    case sasShaderType::Vertex:   DX( _device->CreateVertexShader( shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), 0, &shader.d3dVertexShader ) ); break;
    case sasShaderType::Hull:     DX( _device->CreateHullShader( shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), 0, &shader.d3dHullShader ) ); break;
    case sasShaderType::Domain:   DX( _device->CreateDomainShader( shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), 0, &shader.d3dDomainShader ) ); break;
    case sasShaderType::Geometry: DX( _device->CreateGeometryShader( shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), 0, &shader.d3dGeometryShader ) ); break;
    case sasShaderType::Pixel:    DX( _device->CreatePixelShader( shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), 0, &shader.d3dPixelShader ) ); break;
    case sasShaderType::Compute:  DX( _device->CreateComputeShader( shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), 0, &shader.d3dComputeShader ) ); break;
    default: sasASSERT(0);
    } 
    result = sasNew sasDeviceShader( shaderType, shader.d3dPtr );
  }

  delete [] dxmacros;
  return result;
}

void sasDeviceFactory::releaseShader( sasDeviceObject* obj )
{
  sasDelete (sasDeviceShader*)obj;
}

sasDeviceObject* sasDeviceFactory::createIndirectArgsBuffer( const sasIndirectArgsBuffer& iab )
{
  sasDeviceBuffer* result = 0;  
  ID3D11Buffer* buffer = 0;
  D3D11_BUFFER_DESC desc = {0};  
  ID3D11ShaderResourceView* srv = 0;
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
  HRESULT hr;

  desc.ByteWidth = iab.size();
  desc.Usage = iab.staging() ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT;
  desc.BindFlags = iab.staging() ? 0 : D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = iab.staging() ? D3D11_CPU_ACCESS_READ : 0;
  desc.MiscFlags = iab.staging() ? 0 : D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS | D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

  D3D11_SUBRESOURCE_DATA sd = { 0 };
  sd.pSysMem = iab.buffer();
  DX( _device->CreateBuffer( &desc, &sd, &buffer ) );

  if( !iab.staging() )
  {
    srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    srvDesc.BufferEx.FirstElement = 0;
    srvDesc.BufferEx.NumElements = 4;
    srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
    DX( _device->CreateShaderResourceView( buffer, &srvDesc, &srv ) );
  }
  if SUCCEEDED(hr)
  {
    result = sasNew( sasDeviceBuffer )( buffer, NULL, srv );
  }

  return result;
}

void sasDeviceFactory::releaseIndirectArgsBuffer( sasDeviceObject* obj )
{
  sasDelete (sasDeviceBuffer*)obj;
}

sasDeviceObject* sasDeviceFactory::createConstantBuffer( const sasConstantBuffer& cb )
{
  sasDeviceBuffer* result = 0;  
  ID3D11Buffer* buffer = 0;
  D3D11_BUFFER_DESC desc = {0};  
  HRESULT hr;

  desc.ByteWidth = cb.size();
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  DX( _device->CreateBuffer( &desc, 0, &buffer ) );
  if SUCCEEDED(hr)
  {
    result = sasNew( sasDeviceBuffer )( buffer, NULL, NULL );
  }  
  return result;
}

void sasDeviceFactory::releaseConstantBuffer( sasDeviceObject* obj )
{
  sasDelete (sasDeviceBuffer*)obj;
}

sasDeviceObject* sasDeviceFactory::createStructuredBuffer( const sasStructuredBuffer& sb )
{
  sasDeviceBuffer* result = NULL;  
  ID3D11Buffer* buffer = NULL;
  ID3D11ShaderResourceView* srv = NULL;
  ID3D11UnorderedAccessView* uav = NULL;
  D3D11_BUFFER_DESC desc = {0};  
  HRESULT hr;

  desc.ByteWidth = sb.size();
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  desc.StructureByteStride = sb.structuredByteStride();

  if(!sb.dynamic())
  {
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    desc.CPUAccessFlags = NULL;
  }
  else
  {
    sasASSERT( sb.appendConsume() == false );
  }

  if( sb.staging() )
  { 
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
  }

  DX( _device->CreateBuffer( &desc, 0, &buffer ) );

  if( !sb.staging() )
  {
    D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
    ZeroMemory( &descSRV, sizeof( descSRV ) );
    descSRV.Format = DXGI_FORMAT_UNKNOWN;
    descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    descSRV.Buffer.FirstElement = 0;
    descSRV.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;

    if(FAILED(_device->CreateShaderResourceView(buffer, &descSRV, &srv)))
    {
      DX_RELEASE(buffer);
      return NULL;
    }

    if(!sb.dynamic())
    {
      D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
      ZeroMemory( &descUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC) );
      descUAV.Format = DXGI_FORMAT_UNKNOWN;
      descUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
      descUAV.Buffer.FirstElement = 0;
      descUAV.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;
      descUAV.Buffer.Flags = sb.appendConsume() ? D3D11_BUFFER_UAV_FLAG_APPEND : NULL;
      if(FAILED(_device->CreateUnorderedAccessView(buffer, &descUAV, &uav)))
      {
        DX_RELEASE(srv);
        DX_RELEASE(buffer);
        return NULL;
      }
    }
  }

  if SUCCEEDED(hr)
  {
    result = sasNew( sasDeviceBuffer )( buffer, uav, srv );
  }  
  return result;
}

void sasDeviceFactory::releaseStructuredBuffer( sasDeviceObject* obj )
{
  sasDelete (sasDeviceBuffer*)obj;
}

sasDeviceObject* sasDeviceFactory::createTexture( const sasTexture& tx )
{
  D3D11_TEXTURE2D_DESC desc = {0};
  desc.Width = tx.width();
  desc.Height = tx.height();
  desc.MipLevels = tx.mipCount();
  desc.ArraySize = tx.arraySize();
  desc.Format = sasGetDevicePixelFormat( tx.format() );
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_IMMUTABLE;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  D3D11_SUBRESOURCE_DATA* mipData = new D3D11_SUBRESOURCE_DATA[ tx.mipCount() * tx.arraySize() ];// sasSTACK_ALLOC( D3D11_SUBRESOURCE_DATA, ( tx.mipCount() * tx.arraySize() ) );  
  for( size_t a = 0; a < tx.arraySize(); a++ )
  {
    size_t arraySliceOffset = a * tx.mipCount();
    for( size_t m = 0; m < tx.mipCount(); m++ )
    {
      mipData[ arraySliceOffset + m ].pSysMem      = tx.buffer( a, m );
      mipData[ arraySliceOffset + m ].SysMemPitch  = tx.pitch( m );  
      mipData[ arraySliceOffset + m ].SysMemSlicePitch = 0;
    }
  }

  bool isTexArray = ( tx.arraySize() > 1 );

  ID3D11Texture2D* dxTexture = NULL;
  ID3D11ShaderResourceView* dxShaderResourceView = NULL;
  HRESULT hr;
  DX( _device->CreateTexture2D( &desc, mipData, &dxTexture ) );

  delete [] mipData;

  if( SUCCEEDED( hr ) )
  { 
    if( !isTexArray )
    {
      D3D11_SHADER_RESOURCE_VIEW_DESC descView;
      descView.Format = desc.Format;
      descView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      descView.Texture2D.MostDetailedMip = 0;
      descView.Texture2D.MipLevels = -1;
      DX( _device->CreateShaderResourceView( dxTexture, &descView, &dxShaderResourceView ) );
      if( FAILED( hr ) )
      {
        DX_RELEASE( dxTexture );
      }
    }
    else
    {
      D3D11_SHADER_RESOURCE_VIEW_DESC descView;
      descView.Format = desc.Format;
      descView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
      descView.Texture2DArray.MostDetailedMip = 0;
      descView.Texture2DArray.MipLevels = -1;
      descView.Texture2DArray.FirstArraySlice = 0;
      descView.Texture2DArray.ArraySize = tx.arraySize();
      DX( _device->CreateShaderResourceView( dxTexture, &descView, &dxShaderResourceView ) );
      if( FAILED( hr ) )
      {
        DX_RELEASE( dxTexture );
      }
    }
  }  

  if( !dxTexture )
    return 0;

  return sasNew sasDeviceTexture( dxTexture, NULL, dxShaderResourceView );
}

void sasDeviceFactory::releaseTexture( sasDeviceObject* obj )
{
  sasDelete (sasDeviceTexture*)obj;
}

sasDeviceObject* sasDeviceFactory::createRenderTarget( const sasRenderTarget& rt )
{
  bool isCubeMap = ( rt.type() == sasTextureType::sasTextureCube );
  bool is3D = ( rt.type() == sasTextureType::sasTexture3D );
  if( is3D )
  {
    D3D11_TEXTURE3D_DESC desc = {0};
    desc.Width = rt.width();
    desc.Height = rt.height();
    desc.Depth = rt.depth();
    desc.MipLevels = rt.mipCount();
    desc.Format = sasGetDevicePixelFormat( rt.format() );
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE| D3D11_BIND_RENDER_TARGET;
    if(rt.hasUAV())
      desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

    if(rt.mipCount() != 1)
      desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    ID3D11Texture3D* dxTexture = NULL;
    ID3D11ShaderResourceView* dxShaderResourceView = NULL;
    ID3D11RenderTargetView* dxRTV = NULL;
    ID3D11UnorderedAccessView* dxUAV = NULL;
    HRESULT hr;
    DX( _device->CreateTexture3D( &desc, NULL, &dxTexture ) );

    if SUCCEEDED(hr)
    {    
      DX( _device->CreateShaderResourceView( dxTexture, NULL, &dxShaderResourceView ) );
      if(FAILED( hr ))
      {
        DX_RELEASE( dxTexture );
        return NULL;
      }

      DX( _device->CreateRenderTargetView( dxTexture, NULL, &dxRTV ) );
      if(FAILED(hr))
      {
        DX_RELEASE( dxTexture );
        DX_RELEASE( dxShaderResourceView );
        return NULL;
      }

      if( rt.hasUAV() )
      {
        DX( _device->CreateUnorderedAccessView(dxTexture, NULL, &dxUAV));
        if(FAILED(hr))
        {
          DX_RELEASE( dxTexture );
          DX_RELEASE( dxShaderResourceView );
          DX_RELEASE( dxRTV );
          return NULL;
        }
      }
    }  

    if( !dxTexture )
      return NULL;

    return sasNew sasDeviceRenderTarget( NULL, dxTexture, dxShaderResourceView, dxRTV, dxUAV );

  }
  else //2d or cube rt
  {
    D3D11_TEXTURE2D_DESC desc = {0};
    desc.Width = rt.width();
    desc.Height = rt.height();
    desc.MipLevels = rt.mipCount();
    desc.ArraySize = isCubeMap ? 6 : 1;
    desc.Format = sasGetDevicePixelFormat( rt.format() );
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE| D3D11_BIND_RENDER_TARGET;
    if(rt.hasUAV())
      desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

    if(rt.mipCount() != 1)
      desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    desc.MiscFlags |= isCubeMap ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

    ID3D11Texture2D* dxTexture = NULL;
    ID3D11ShaderResourceView* dxShaderResourceView = NULL;
    ID3D11RenderTargetView* dxRTV = NULL;
    ID3D11UnorderedAccessView* dxUAV = NULL;
    HRESULT hr;
    DX( _device->CreateTexture2D( &desc, NULL, &dxTexture ) );

    if SUCCEEDED(hr)
    {    
      D3D11_SHADER_RESOURCE_VIEW_DESC descView;
      descView.Format = desc.Format;
      descView.ViewDimension = isCubeMap ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;
      if( isCubeMap )
      {
        descView.TextureCube.MostDetailedMip = 0;
        descView.TextureCube.MipLevels = -1;
      }
      else
      {
        descView.Texture2D.MostDetailedMip = 0;
        descView.Texture2D.MipLevels = -1;
      }
      DX( _device->CreateShaderResourceView( dxTexture, &descView, &dxShaderResourceView ) );
      if(FAILED( hr ))
      {
        DX_RELEASE( dxTexture );
        return NULL;
      }

      D3D11_RENDER_TARGET_VIEW_DESC descRTV;
      descRTV.Format = desc.Format;
      descRTV.ViewDimension = isCubeMap ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY : D3D11_RTV_DIMENSION_TEXTURE2D;
      if( isCubeMap )
      {
        descRTV.Texture2DArray.FirstArraySlice = 0;
        descRTV.Texture2DArray.ArraySize = 6;
        descRTV.Texture2DArray.MipSlice = 0;
      }
      else
      {
        descRTV.Texture2D.MipSlice = 0;
      }
      DX( _device->CreateRenderTargetView(dxTexture, &descRTV, &dxRTV));
      if(FAILED(hr))
      {
        DX_RELEASE( dxTexture );
        DX_RELEASE( dxShaderResourceView );
        return NULL;
      }

      if( rt.hasUAV() )
      {
        sasASSERT( rt.type() != sasTextureType::sasTextureCube );

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        uavDesc.Format = desc.Format;
        uavDesc.Texture2D.MipSlice = 0;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        DX( _device->CreateUnorderedAccessView(dxTexture, &uavDesc, &dxUAV));
        if(FAILED(hr))
        {
          DX_RELEASE( dxTexture );
          DX_RELEASE( dxShaderResourceView );
          DX_RELEASE( dxRTV );
          return NULL;
        }
      }
    }  

    if( !dxTexture )
      return NULL;

    return sasNew sasDeviceRenderTarget( dxTexture, NULL, dxShaderResourceView, dxRTV, dxUAV );
  }
  
}

void sasDeviceFactory::releaseRenderTarget( sasDeviceObject* obj )
{
  sasDelete (sasDeviceRenderTarget*)obj;
}

sasDeviceObject* sasDeviceFactory::createDepthStencilTarget( const sasDepthStencilTarget& dst )
{
  bool isCubeMap = ( dst.type() == sasTextureType::sasTextureCube );
  D3D11_TEXTURE2D_DESC desc = {0};
  desc.Width = dst.width();
  desc.Height = dst.height();
  desc.MipLevels = 1;
  desc.ArraySize = isCubeMap ? 6 : 1;
  desc.Format = sasGetDevicePixelFormatTypeless( dst.format() );
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE| D3D11_BIND_DEPTH_STENCIL;
  desc.MiscFlags = isCubeMap ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

  ID3D11Texture2D* dxTexture = NULL;
  ID3D11ShaderResourceView* dxShaderResourceView = NULL;
  ID3D11DepthStencilView* dxDSV = NULL;
  ID3D11DepthStencilView* dxReadOnlyDSV = NULL;
  HRESULT hr;
  DX(_device->CreateTexture2D( &desc, NULL, &dxTexture ) );

  if SUCCEEDED(hr)
  {    
    D3D11_SHADER_RESOURCE_VIEW_DESC descView;
    descView.Format = desc.Format = sasGetDeviceDepthFormatSRV( dst.format() );
    descView.ViewDimension = isCubeMap ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;
    if( isCubeMap )
    {
      descView.TextureCube.MostDetailedMip = 0;
      descView.TextureCube.MipLevels = -1;
    }
    else
    {
      descView.Texture2D.MostDetailedMip = 0;
      descView.Texture2D.MipLevels = -1;
    }
    DX( _device->CreateShaderResourceView( dxTexture, &descView, &dxShaderResourceView ) );
    if(FAILED( hr ))
    {
      DX_RELEASE( dxTexture );
      return NULL;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    descDSV.Flags = 0;
    descDSV.Format = sasGetDevicePixelFormat( dst.format() );
    descDSV.ViewDimension = isCubeMap ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D;
    if( isCubeMap )
    {
      descDSV.Texture2DArray.FirstArraySlice = 0;
      descDSV.Texture2DArray.ArraySize = 6;
      descDSV.Texture2DArray.MipSlice = 0;
    }
    else
    {
      descDSV.Texture2D.MipSlice = 0;
    }
    DX( _device->CreateDepthStencilView(dxTexture, &descDSV, &dxDSV));
    if(FAILED(hr))
    {
      DX_RELEASE( dxTexture );
      DX_RELEASE( dxShaderResourceView );
      return NULL;
    }

    descDSV.Flags = ( dst.format() == sasPixelFormat::D24S8 ) ? ( D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL ) : D3D11_DSV_READ_ONLY_DEPTH;
    DX( _device->CreateDepthStencilView(dxTexture, &descDSV, &dxReadOnlyDSV));
    if(FAILED(hr))
    {
      DX_RELEASE( dxTexture );
      DX_RELEASE( dxShaderResourceView );
      DX_RELEASE( dxDSV );
      return NULL;
    }

  }  

  if( !dxTexture )
    return NULL;

  return sasNew sasDeviceDepthStencilTarget(  dxTexture, dxShaderResourceView, dxDSV, dxReadOnlyDSV );
}

void sasDeviceFactory::releaseDepthStencilTarget( sasDeviceObject* obj )
{
  sasDelete (sasDeviceDepthStencilTarget*)obj;
}

sasDeviceObject* sasDeviceFactory::createGPUTimer( const sasGPUTimer& gpuTimer )
{
  sasDeviceGPUTimer* result = NULL;  
  ID3D11Query* startQuery = NULL;
  ID3D11Query* endQuery = NULL;
  ID3D11Query* disjointQuery = NULL;
  HRESULT hr;

  D3D11_QUERY_DESC desc;
  desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
  desc.MiscFlags = 0;
  DX( _device->CreateQuery( &desc, &disjointQuery ) );

  desc.Query = D3D11_QUERY_TIMESTAMP;
  DX( _device->CreateQuery( &desc, &startQuery ) );
  DX( _device->CreateQuery( &desc, &endQuery ) );

  if SUCCEEDED(hr)
  {
    result = sasNew( sasDeviceGPUTimer )( disjointQuery, startQuery, endQuery );
  }  
  return result;
}

void sasDeviceFactory::releaseGPUTimer( sasDeviceObject* obj )
{
  sasDelete (sasDeviceGPUTimer*)obj;
}
