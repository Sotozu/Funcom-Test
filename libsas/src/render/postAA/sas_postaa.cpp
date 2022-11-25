#include "sas_pch.h"
#include "sas_postaa.h"
#include "render/shaders/sas_shaderids.h"
#include "render/sas_rendertarget.h"
#include "render/sas_constantbuffer.h"
#include "render/sas_miscresources.h"
#include "sas_device.h"
#include "render/sas_rendertargetpool.h"
#include "render/sas_rendermng.h"

struct AAConstants
{
  smVec4  invBufferSize_bufferSize;
};

sasPostAA::sasPostAA()
{
  _constants = sasNew sasConstantBuffer( sizeof( AAConstants ) );
}

sasPostAA::~sasPostAA()
{
  sasDelete _constants;
}

void sasPostAA::computePostAA( sasRenderTarget* imageBuffer, sasRenderTarget* destBuffer )
{
  INSERT_PIX_EVENT("PIXEvent: fxaa pass", 0xffffffff);

  AAConstants* constantBuffer = static_cast<AAConstants*>( _constants->map() );
  float w = static_cast< float >( imageBuffer->width() );
  float h = static_cast< float >( imageBuffer->height() );
  constantBuffer->invBufferSize_bufferSize = smVec4( 1.f / w, 1.f / h, w, h );
  _constants->unmap();

  sasDevice* device = sasDevice::singletonPtr();
  sasRenderTargetDesc rtDesc;
  rtDesc.width = imageBuffer->width();
  rtDesc.height = imageBuffer->height();
  rtDesc.format = sasPixelFormat::RGBA_U8;
  rtDesc.mips = 1;
  rtDesc.needUAV = false;
  sasRenderTarget* tempBuffer = sasRenderTargetPool::singletonPtr()->RequestAndLock( rtDesc );

  device->setRenderTarget( 0, tempBuffer );
  device->flushStates();

  device->setConstantBuffer( 1, _constants, sasDeviceUnit::PixelShader );
  device->setVertexShader( fxaaShaderID );
  device->setPixelShader( fxaaShaderID );
  device->setVertexBuffer( sasMiscResources::singletonPtr()->getFullScreenQuadVB() );
  device->setTexture( 0, imageBuffer, sasDeviceUnit::PixelShader );
  device->setSamplerState( 0, sasSamplerState_Bilinear_Clamp, sasDeviceUnit::PixelShader );
  device->setViewport( destBuffer );
  device->setDepthStencilTarget( NULL, false );
  device->draw( sasPrimTopology_TriStrip, 4 );
  device->setTexture( 0, static_cast< sasTexture* >( NULL ), sasDeviceUnit::PixelShader );
  device->setRenderTarget( 0, NULL );

  device->setRenderTarget( 0, static_cast<sasRenderTarget*>(NULL) );
  device->setTexture( 0, static_cast<sasTexture*>(NULL), sasDeviceUnit::PixelShader );
  device->flushStates();

  sasRenderMng::singletonPtr()->copyRenderTarget( tempBuffer, destBuffer );

  sasRenderTargetPool::singletonPtr()->Unlock( tempBuffer );
}

