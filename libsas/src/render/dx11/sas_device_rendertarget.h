#pragma once

#include "sas_device_object.h"
#include "sas_device_texture.h"

class sasDeviceTexture;

class sasDeviceRenderTarget : public sasDeviceObject
{
  sasNO_COPY( sasDeviceRenderTarget );
  sasMEM_OP( sasDeviceRenderTarget );

public:
  sasDeviceRenderTarget( ID3D11Texture2D* pTex, ID3D11Texture3D* pTex3D, ID3D11ShaderResourceView* pSRV, ID3D11RenderTargetView* pRTV, ID3D11UnorderedAccessView* pUAV );
  ~sasDeviceRenderTarget();

  size_t                      width() const { return _texture->width(); }
  size_t                      height() const { return _texture->height(); }

  sasDeviceTexture*           textureObj() const { return _texture; }   
  ID3D11RenderTargetView*     d3dRTV() const { return _rtv; } 
  ID3D11UnorderedAccessView*  d3dUAV() const { return _uav; } 

private:
  sasDeviceTexture* _texture;
  ID3D11RenderTargetView* _rtv;
  ID3D11UnorderedAccessView* _uav;
};
