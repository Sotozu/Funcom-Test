#include "sas_pch.h"
#include "sas_volumetricLight.h"
#include "sas_device.h"
#include "../shaders/sas_shaderids.h"
#include "../shaders/sas_shadermng.h"
#include "../sas_rendertarget.h"
#include "../sas_depthbuffer.h"
#include "../sas_constantbuffer.h"
#include "../sas_rendertargetpool.h"
#include "../sas_rendermng.h"

sasVolumetricLightMng::sasVolumetricLightMng()
{

}

sasVolumetricLightMng::~sasVolumetricLightMng()
{

}

void sasVolumetricLightMng::performVolumetricLightPass( sasRenderTarget* lightBuffer, sasDepthStencilTarget* depthBuffer )
{
  sasDevice* device = sasDevice::singletonPtr();
  sasRenderTargetDesc rtDesc;
  rtDesc.width = lightBuffer->width();
  rtDesc.height = lightBuffer->height();
  rtDesc.format = sasPixelFormat::RGBA_U8;
  rtDesc.mips = 1;
  rtDesc.needUAV = true;
  sasRenderTarget* godrayBuffer = sasRenderTargetPool::singletonPtr()->RequestAndLock( rtDesc );

  //device->setConstantBuffer( 1, _constantBuffer, sasDeviceUnit::ComputeShader );
  device->setTexture( 0, depthBuffer, sasDeviceUnit::ComputeShader );
  device->setTexture( 1, lightBuffer, sasDeviceUnit::ComputeShader );
  device->setComputeShader( volumetricLightShaderID );
  device->setUAV( 0, godrayBuffer, sasDeviceUnit::ComputeShader );

  size_t nGroupX = (godrayBuffer->width() + 15 ) / 16;
  size_t nGroupY = (godrayBuffer->height() + 15 ) / 16;
  device->dispatch( nGroupX, nGroupY, 1 );

  

  device->setUAV( 0, static_cast<sasRenderTarget*>(NULL), sasDeviceUnit::ComputeShader );
  device->setTexture( 0, static_cast<sasTexture*>(NULL), sasDeviceUnit::ComputeShader );
  device->setTexture( 1, static_cast<sasTexture*>(NULL), sasDeviceUnit::ComputeShader );
  device->flushStates();

  sasRenderMng::singletonPtr()->copyRenderTarget( godrayBuffer, device->backBuffer() );

  sasRenderTargetPool::singletonPtr()->Unlock( godrayBuffer );
}