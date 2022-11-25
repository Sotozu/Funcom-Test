#include "sas_pch.h"
#include "sas_device_typeconversion.h"

void sasGetDeviceDesc(sasBlendState type, D3D11_BLEND_DESC& outDesc)
{
  D3D11_BLEND_DESC d3d11Desc = { 0 };

  switch (type)
  {
  case sasBlendState_Opaque:
    d3d11Desc.RenderTarget[0].BlendEnable = FALSE;
    d3d11Desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    break;
  case sasBlendState_Opaque_NoColourWrites:
    d3d11Desc.RenderTarget[0].BlendEnable = FALSE;
    d3d11Desc.RenderTarget[0].RenderTargetWriteMask = 0;
    break;
  case sasBlendState_One_One:
    d3d11Desc.RenderTarget[0].BlendEnable = TRUE;
    d3d11Desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    d3d11Desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    d3d11Desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    d3d11Desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    d3d11Desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    d3d11Desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    d3d11Desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    break;
  case sasBlendState_AlphaInvAlpha:
    d3d11Desc.RenderTarget[0].BlendEnable = TRUE;
    d3d11Desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    d3d11Desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    d3d11Desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    d3d11Desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    d3d11Desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    d3d11Desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    d3d11Desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    break;
  default:
    sasASSERT(false);
  }

  outDesc = d3d11Desc;
}

void sasGetDeviceDesc(sasDepthStencilState type, D3D11_DEPTH_STENCIL_DESC& outDesc)
{
  D3D11_DEPTH_STENCIL_DESC d3d11Desc = { 0 };  

  switch(type)
  {
  case sasDepthStencilState_LessEqual:
    d3d11Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    d3d11Desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    d3d11Desc.DepthEnable = TRUE;
    break;

  case sasDepthStencilState_NoZTest:
    {
      d3d11Desc.DepthEnable = FALSE;
      d3d11Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
      d3d11Desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    }
    break;

  case sasDepthStencilState_NoZWrite_Equal:
    {
      d3d11Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
      d3d11Desc.DepthFunc = D3D11_COMPARISON_EQUAL;
      d3d11Desc.DepthEnable = TRUE;
    }
    break;

  case sasDepthStencilState_NoZWrite_Greater:
    {
      d3d11Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
      d3d11Desc.DepthFunc = D3D11_COMPARISON_GREATER;
      d3d11Desc.DepthEnable = TRUE;
    }
    break;

  case sasDepthStencilState_NoZWrite_Less:
    {
      d3d11Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
      d3d11Desc.DepthFunc = D3D11_COMPARISON_LESS;
      d3d11Desc.DepthEnable = TRUE;
    }
    break;

  case sasDepthStencilState_NoZWrite_LessEqual:
    {
      d3d11Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
      d3d11Desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
      d3d11Desc.DepthEnable = TRUE;
    }
    break;

  case sasDepthStencilState_NoZWrite_NoZTest:
    {
      d3d11Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
      d3d11Desc.DepthEnable = FALSE;
    }
    break;
    
  default:
    sasASSERT(false);
    break;
  }

  outDesc = d3d11Desc;
}

void sasGetDeviceDesc(sasSamplerState type, D3D11_SAMPLER_DESC& outDesc)
{
  D3D11_SAMPLER_DESC d3d11Desc;
  memset(&d3d11Desc, 0, sizeof(d3d11Desc));
  d3d11Desc.MinLOD = -FLT_MAX;
  d3d11Desc.MaxLOD = FLT_MAX;
  switch(type)
  {
  case sasSamplerState_Bilinear_Clamp:
    {
      d3d11Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
      d3d11Desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
      d3d11Desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
      d3d11Desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    }
    break;

  case sasSamplerState_Bilinear_Wrap:
    {
      d3d11Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
      d3d11Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
      d3d11Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
      d3d11Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    }
    break;

  case sasSamplerState_Linear_ShadowCompare:
    {
      d3d11Desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
      d3d11Desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
      d3d11Desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
      d3d11Desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
      d3d11Desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
      FLOAT borderCol[4] = {0.f, 0.f, 0.f, 0.f};
      memcpy(d3d11Desc.BorderColor, borderCol, sizeof(borderCol));
      d3d11Desc.MinLOD = 0;
      d3d11Desc.MaxLOD = 0;
    }
    break;

  case sasSamplerState_Point_Clamp:
    {
      d3d11Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
      d3d11Desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
      d3d11Desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
      d3d11Desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    }
    break;

  case sasSamplerState_Point_Wrap:
    {
      d3d11Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
      d3d11Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
      d3d11Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
      d3d11Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    }
    break;

  case sasSamplerState_Point_Border:
    {
      d3d11Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
      d3d11Desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
      d3d11Desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
      d3d11Desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    }
    break;

  case sasSamplerState_Point_ShadowCompare:
    {
      d3d11Desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
      d3d11Desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    }
    break;

  case sasSamplerState_Anisotropic_Clamp:
    {
      d3d11Desc.Filter = D3D11_FILTER_ANISOTROPIC;
      d3d11Desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
      d3d11Desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
      d3d11Desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
      d3d11Desc.MaxAnisotropy = 16;
    }
    break;

  case sasSamplerState_Anisotropic_Wrap:
    {
      d3d11Desc.Filter = D3D11_FILTER_ANISOTROPIC;
      d3d11Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
      d3d11Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
      d3d11Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
      d3d11Desc.MaxAnisotropy = 16;
    }
    break;

  default:
    sasASSERT(false);
    break;
  }

  outDesc = d3d11Desc;
}

void sasGetDeviceDesc(sasRasterizerState type, D3D11_RASTERIZER_DESC& outDesc)
{
  D3D11_RASTERIZER_DESC d3d11Desc;
  memset(&d3d11Desc, 0, sizeof(d3d11Desc));  

  switch (type)
  {
  case sasRasterizerState_Solid:
    {
      d3d11Desc.FillMode = D3D11_FILL_SOLID;
      d3d11Desc.CullMode = D3D11_CULL_BACK;
      d3d11Desc.FrontCounterClockwise = TRUE;
      d3d11Desc.DepthClipEnable = TRUE;
    }
    break;
  case sasRasterizerState_Solid_NoCull:
    {
      d3d11Desc.FillMode = D3D11_FILL_SOLID;
      d3d11Desc.CullMode = D3D11_CULL_NONE;
      d3d11Desc.FrontCounterClockwise = TRUE;
      d3d11Desc.DepthClipEnable = TRUE;
    }
    break;
  
  case sasRasterizerState_Wireframe:
    {
      d3d11Desc.FillMode = D3D11_FILL_WIREFRAME;
      d3d11Desc.CullMode = D3D11_CULL_BACK;
      d3d11Desc.FrontCounterClockwise = TRUE;
      d3d11Desc.DepthClipEnable = TRUE;
    }
    break;

  case sasRasterizerState_Wireframe_NoCull:
    {
      d3d11Desc.FillMode = D3D11_FILL_WIREFRAME;
      d3d11Desc.CullMode = D3D11_CULL_NONE;
      d3d11Desc.FrontCounterClockwise = TRUE;
      d3d11Desc.DepthClipEnable = TRUE;
    }
    break;

  default:
    sasASSERT(false);
  }

  outDesc = d3d11Desc;
}

DXGI_FORMAT sasGetDevicePixelFormat( sasPixelFormat::Enum pf )
{
  switch( pf )
  {
  case sasPixelFormat::BGRA_U8: return DXGI_FORMAT_B8G8R8A8_UNORM;
  case sasPixelFormat::DXT1:    return DXGI_FORMAT_BC1_UNORM;
  case sasPixelFormat::DXT5:    return DXGI_FORMAT_BC3_UNORM;
  case sasPixelFormat::RGBA_U8: return DXGI_FORMAT_R8G8B8A8_UNORM;
  case sasPixelFormat::RGBA_F16:return DXGI_FORMAT_R16G16B16A16_FLOAT;
  case sasPixelFormat::RGBA_F32:return DXGI_FORMAT_R32G32B32A32_FLOAT;
  case sasPixelFormat::RG_F16:  return DXGI_FORMAT_R16G16_FLOAT;
  case sasPixelFormat::R_F16:   return DXGI_FORMAT_R16_FLOAT;
  case sasPixelFormat::R_F32:   return DXGI_FORMAT_R32_FLOAT;
  case sasPixelFormat::D16:     return DXGI_FORMAT_D16_UNORM;
  case sasPixelFormat::D32:     return DXGI_FORMAT_D32_FLOAT;
  case sasPixelFormat::D24S8:   return DXGI_FORMAT_D24_UNORM_S8_UINT;
  }

  sasASSERT( 0 );
  return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT sasGetDevicePixelFormatTypeless( sasPixelFormat::Enum pf )
{
  switch( pf )
  {
  case sasPixelFormat::BGRA_U8: return DXGI_FORMAT_B8G8R8A8_TYPELESS;
  case sasPixelFormat::DXT1:    return DXGI_FORMAT_BC1_TYPELESS;
  case sasPixelFormat::DXT5:    return DXGI_FORMAT_BC3_TYPELESS;
  case sasPixelFormat::RGBA_U8: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
  case sasPixelFormat::RGBA_F16:return DXGI_FORMAT_R16G16B16A16_TYPELESS;
  case sasPixelFormat::RGBA_F32:return DXGI_FORMAT_R32G32B32A32_TYPELESS;
  case sasPixelFormat::RG_F16:  return DXGI_FORMAT_R16G16_TYPELESS;
  case sasPixelFormat::R_F16:   return DXGI_FORMAT_R16_TYPELESS;
  case sasPixelFormat::R_F32:   return DXGI_FORMAT_R32_TYPELESS;
  case sasPixelFormat::D16:     return DXGI_FORMAT_R16_TYPELESS;
  case sasPixelFormat::D32:     return DXGI_FORMAT_R32_TYPELESS;
  case sasPixelFormat::D24S8:   return DXGI_FORMAT_R24G8_TYPELESS;
  }

  sasASSERT( 0 );
  return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT sasGetDeviceDepthFormatSRV( sasPixelFormat::Enum pf )
{
  switch( pf )
  {
  case sasPixelFormat::D16:     return DXGI_FORMAT_R16_UNORM;
  case sasPixelFormat::D32:     return DXGI_FORMAT_R32_FLOAT;
  case sasPixelFormat::D24S8:   return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  }

  sasASSERT( 0 );
  return DXGI_FORMAT_UNKNOWN;
}

void sasGetDeviceViewport( const sasViewport& sasVp, D3D11_VIEWPORT* dxVp)
{
  dxVp->Height = sasVp.height;
  dxVp->Width = sasVp.width;
  dxVp->MinDepth = sasVp.minDepth;
  dxVp->MaxDepth = sasVp.maxDepth;
  dxVp->TopLeftX = sasVp.topLeftX;
  dxVp->TopLeftY = sasVp.topLeftY;
}

UINT sasGetDeviceClearFlags(UINT sasClearFlags)
{
  return sasClearFlags;
}

D3D11_PRIMITIVE_TOPOLOGY sasGetDevicePrimTopology( sasPrimTopology primTopology )
{
  switch( primTopology )
  {
  case sasPrimTopology_TriList:     return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
  case sasPrimTopology_TriStrip:    return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
  case sasPrimTopology_TriPatch:    return D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
  case sasPrimTopology_PointList:   return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
  case sasPrimTopology_LineList:    return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
  }

  sasASSERT( 0 );
  return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

