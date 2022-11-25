#include "sas_pch.h"
#include "sas_device_statebuffercache.h"
#include "sas_device_common.h"
#include "sas_device_typeconversion.h"

sasDeviceStateBufferCache::sasDeviceStateBufferCache(ID3D11Device* device)
  : _device( device )
{
  memset(_samplerStateArray, NULL, sizeof(_samplerStateArray));
  memset(_rasterizerStateArray, NULL, sizeof(_rasterizerStateArray));
  memset(_depthStencilStateArray, NULL, sizeof(_depthStencilStateArray));
  memset(_blendStateArray, NULL, sizeof(_blendStateArray));
}

sasDeviceStateBufferCache::~sasDeviceStateBufferCache()
{
  clear();
}

void sasDeviceStateBufferCache::clear()
{
  for(unsigned int i = 0; i < sasSamplerStateCount; i++)
  {
    DX_RELEASE(_samplerStateArray[i]);
  }

  for(unsigned int i = 0; i < sasRasterizerStateCount; i++)
  {
    DX_RELEASE(_rasterizerStateArray[i]);
  }

  for(unsigned int i = 0; i < sasDepthStencilStateCount; i++)
  {
    DX_RELEASE(_depthStencilStateArray[i]);
  }

  for(unsigned int i = 0; i < sasBlendStateCount; i++)
  {
    DX_RELEASE(_blendStateArray[i]);
  }
}

ID3D11RasterizerState* sasDeviceStateBufferCache::requestRasterizerState(sasRasterizerState type)
{
  if(_rasterizerStateArray[type] == NULL)
  {
    D3D11_RASTERIZER_DESC desc;
    sasGetDeviceDesc(type, desc);
    if(FAILED(_device->CreateRasterizerState(&desc, &_rasterizerStateArray[type])))
    {
      sasASSERT(false);
    }
  }
  return _rasterizerStateArray[type];
}

ID3D11BlendState* sasDeviceStateBufferCache::requestBlendState(sasBlendState type)
{
  if(_blendStateArray[type] == NULL)
  {
    D3D11_BLEND_DESC desc;
    sasGetDeviceDesc(type, desc);
    if(FAILED(_device->CreateBlendState(&desc, &_blendStateArray[type])))
    {
      sasASSERT(false);
    }
  }
  return _blendStateArray[type];
}

ID3D11DepthStencilState* sasDeviceStateBufferCache::requestDepthStencilState(sasDepthStencilState type)
{
  if(_depthStencilStateArray[type] == NULL)
  {
    D3D11_DEPTH_STENCIL_DESC desc;
    sasGetDeviceDesc(type, desc);
    if(FAILED(_device->CreateDepthStencilState(&desc, &_depthStencilStateArray[type])))
    {
      sasASSERT(false);
    }
  }
  return _depthStencilStateArray[type];
}

ID3D11SamplerState* sasDeviceStateBufferCache::requestSamplerState(sasSamplerState type)
{
  if(_samplerStateArray[type] == NULL)
  {
    D3D11_SAMPLER_DESC desc;
    sasGetDeviceDesc(type, desc);
    if(FAILED(_device->CreateSamplerState(&desc, &_samplerStateArray[type])))
    {
      sasASSERT(false);
    }
  }
  return _samplerStateArray[type];
}
