#pragma once
#include "render/sas_rendertypes.h"

struct sasViewport;

void sasGetDeviceDesc(sasBlendState type, D3D11_BLEND_DESC& outDesc);
void sasGetDeviceDesc(sasDepthStencilState type, D3D11_DEPTH_STENCIL_DESC& outDesc);
void sasGetDeviceDesc(sasSamplerState type, D3D11_SAMPLER_DESC& outDesc);
void sasGetDeviceDesc(sasRasterizerState type, D3D11_RASTERIZER_DESC& outDesc);

DXGI_FORMAT sasGetDevicePixelFormat( sasPixelFormat::Enum pf );
DXGI_FORMAT sasGetDevicePixelFormatTypeless( sasPixelFormat::Enum pf );
DXGI_FORMAT sasGetDeviceDepthFormatSRV( sasPixelFormat::Enum pf );

void sasGetDeviceViewport( const sasViewport& sasVp, D3D11_VIEWPORT* dxVp );

UINT sasGetDeviceClearFlags(UINT sasClearFlags);

D3D11_PRIMITIVE_TOPOLOGY sasGetDevicePrimTopology( sasPrimTopology primTopology );