#pragma once

#include "sas_device_object.h"

class sasDeviceTexture : public sasDeviceObject
{
  sasNO_COPY( sasDeviceTexture );
  sasMEM_OP( sasDeviceTexture );

public:
  sasDeviceTexture( ID3D11Texture2D* tx, ID3D11Texture3D* tx3D, ID3D11ShaderResourceView* srv );
  ~sasDeviceTexture();
 
  size_t                    width() const;
  size_t                    height() const;
  size_t                    depth() const;
  ID3D11ShaderResourceView* d3dShaderResourceView() const { return _shaderResourceView; }
  ID3D11Texture2D*          d3dTexture() const { return _texture; }
  ID3D11Texture3D*          d3dTexture3D() const { return _texture3D; }
  bool                      isVolumeTexture() const { return _texture3D != NULL; }

private:
  ID3D11Texture2D*          _texture;
  ID3D11Texture3D*          _texture3D;
  ID3D11ShaderResourceView* _shaderResourceView;
};
