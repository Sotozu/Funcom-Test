#include "sas_pch.h"
#include "sas_device_texture.h"
#include "sas_device_common.h"

sasDeviceTexture::sasDeviceTexture( ID3D11Texture2D* tx, ID3D11Texture3D* tx3D, ID3D11ShaderResourceView* srv )
  : sasDeviceObject( sasDeviceObjectType::Texture )
  , _texture( tx )
  , _texture3D( tx3D )
  , _shaderResourceView( srv )
{
}

sasDeviceTexture::~sasDeviceTexture()
{
  DX_RELEASE( _shaderResourceView );
  DX_RELEASE( _texture );
  DX_RELEASE( _texture3D );
}

size_t sasDeviceTexture::width() const
{
  if( _texture )
  {
    D3D11_TEXTURE2D_DESC texDesc;
    _texture->GetDesc(&texDesc);
    return texDesc.Width;
  }

  if( _texture3D )
  {
    D3D11_TEXTURE3D_DESC texDesc;
    _texture3D->GetDesc(&texDesc);
    return texDesc.Width;
  }

  sasASSERT( false );
  return 0;
}

size_t sasDeviceTexture::height() const
{
  if( _texture )
  {
    D3D11_TEXTURE2D_DESC texDesc;
    _texture->GetDesc(&texDesc);
    return texDesc.Height;
  }

  if( _texture3D )
  {
    D3D11_TEXTURE3D_DESC texDesc;
    _texture3D->GetDesc(&texDesc);
    return texDesc.Height;
  }

  sasASSERT( false );
  return 0;
}

size_t sasDeviceTexture::depth() const
{
  if( _texture3D )
  {
    D3D11_TEXTURE3D_DESC texDesc;
    _texture3D->GetDesc(&texDesc);
    return texDesc.Depth;
  }

  sasASSERT( false );
  return 0;
}

