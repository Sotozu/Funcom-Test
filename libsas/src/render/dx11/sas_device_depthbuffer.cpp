#include "sas_pch.h"
#include "sas_device_depthbuffer.h"
#include "sas_device_texture.h"
#include "sas_device_common.h"

sasDeviceDepthStencilTarget::sasDeviceDepthStencilTarget( ID3D11Texture2D* pTex, ID3D11ShaderResourceView* pSRV, ID3D11DepthStencilView* pDSV, ID3D11DepthStencilView* pReadOnlyDSV )
  : sasDeviceObject( sasDeviceObjectType::RenderTarget )
  , _dsv( pDSV )
  , _readOnlyDsv( pReadOnlyDSV )
{
  _texture = sasNew sasDeviceTexture( pTex, NULL, pSRV );
}

sasDeviceDepthStencilTarget::~sasDeviceDepthStencilTarget()
{
  DX_RELEASE( _dsv );
  DX_RELEASE( _readOnlyDsv );
  sasDelete _texture;
}