#pragma once

#include "sas_device_object.h"

class sasDeviceTexture;

class sasDeviceDepthStencilTarget : public sasDeviceObject
{
  sasNO_COPY( sasDeviceDepthStencilTarget );
  sasMEM_OP( sasDeviceDepthStencilTarget );

public:
  sasDeviceDepthStencilTarget( ID3D11Texture2D* pTex, ID3D11ShaderResourceView* pSRV, ID3D11DepthStencilView* pDSV, ID3D11DepthStencilView* pReadOnlyDSV );
  ~sasDeviceDepthStencilTarget();

  sasDeviceTexture* textureObj() const { return _texture; }   
  ID3D11DepthStencilView* d3dDSV() const { return _dsv; } 
  ID3D11DepthStencilView* d3dReadOnlyDSV() const { return _readOnlyDsv; } 

private:
  sasDeviceTexture* _texture;
  ID3D11DepthStencilView* _dsv;
  ID3D11DepthStencilView* _readOnlyDsv;
};
