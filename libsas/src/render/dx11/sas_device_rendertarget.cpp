#include "sas_pch.h"
#include "sas_device_rendertarget.h"
#include "sas_device_texture.h"
#include "sas_device_common.h"

sasDeviceRenderTarget::sasDeviceRenderTarget( ID3D11Texture2D* pTex, ID3D11Texture3D* pTex3D, ID3D11ShaderResourceView* pSRV, ID3D11RenderTargetView* pRTV, ID3D11UnorderedAccessView* pUAV )
  : sasDeviceObject( sasDeviceObjectType::RenderTarget )
  , _rtv( pRTV )
  , _uav( pUAV )
{
  _texture = sasNew sasDeviceTexture( pTex, pTex3D, pSRV );
}

sasDeviceRenderTarget::~sasDeviceRenderTarget()
{
  DX_RELEASE( _rtv );
  DX_RELEASE( _uav );
  sasDelete _texture;
}

