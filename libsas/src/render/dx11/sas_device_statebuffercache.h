#pragma once

#include "render/sas_rendertypes.h"
#include "utils/sas_singleton.h"

class sasDeviceStateBufferCache : public sasSingleton<sasDeviceStateBufferCache>
{
  sasNO_COPY( sasDeviceStateBufferCache );
  sasMEM_OP( sasDeviceStateBufferCache );

public: 
  sasDeviceStateBufferCache(ID3D11Device* device);
  ~sasDeviceStateBufferCache();

  ID3D11BlendState*         requestBlendState(sasBlendState type);
  ID3D11DepthStencilState*  requestDepthStencilState(sasDepthStencilState type);
  ID3D11RasterizerState*    requestRasterizerState(sasRasterizerState type);
  ID3D11SamplerState*       requestSamplerState(sasSamplerState type);

  void clear();

private:
  ID3D11BlendState*         _blendStateArray[sasBlendStateCount];
  ID3D11DepthStencilState*  _depthStencilStateArray[sasDepthStencilStateCount];
  ID3D11RasterizerState*    _rasterizerStateArray[sasRasterizerStateCount];
  ID3D11SamplerState*       _samplerStateArray[sasSamplerStateCount];

  ID3D11Device* _device;
};

