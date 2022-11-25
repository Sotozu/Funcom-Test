#include "sas_pch.h"
#include "sas_motionBlur.h"
#include "sas_device.h"
#include "../shaders/sas_shaderids.h"
#include "../shaders/sas_shadermng.h"
#include "../sas_rendertarget.h"
#include "../sas_depthbuffer.h"
#include "../sas_constantbuffer.h"
#include "../sas_rendertargetpool.h"
#include "../sas_rendermng.h"
#include "../sas_miscresources.h"
#include "../sas_settings.h"
#include "utils/sas_timer.h"

sasMotionBlur::sasMotionBlur()
{
   _constantBuffer = sasNew sasConstantBuffer( sizeof(float) * 4 );
}

sasMotionBlur::~sasMotionBlur()
{
  sasDelete _constantBuffer;
}

void sasMotionBlur::performMotionBlurPass( sasRenderTarget* colourBuffer, sasRenderTarget* velocityBuffer, sasRenderTarget* outputBuffer, sasDepthStencilTarget* depthBuffer )
{
  sasDevice* device = sasDevice::singletonPtr();
  sasRenderTargetDesc rtDesc;
  rtDesc.width = colourBuffer->width();
  rtDesc.height = colourBuffer->height();
  rtDesc.format = sasPixelFormat::RGBA_U8;
  rtDesc.mips = 1;
  rtDesc.needUAV = false;
  sasRenderTarget* tempBuffer = sasRenderTargetPool::singletonPtr()->RequestAndLock( rtDesc );

  smVec4* constantBuffer = static_cast<smVec4*>(_constantBuffer->map());
  *constantBuffer = smVec4( sasSettings::singletonPtr()->_motionBlurIntensity, sasTimer::singletonPtr()->getCachedFPS(), 0.f, 0.f );
  _constantBuffer->unmap();

  device->setConstantBuffer( 1, _constantBuffer, sasDeviceUnit::PixelShader );
  device->setTexture( 0, depthBuffer, sasDeviceUnit::PixelShader );
  device->setSamplerState( 0, sasSamplerState_Point_Clamp, sasDeviceUnit::PixelShader );
  device->setTexture( 1, colourBuffer, sasDeviceUnit::PixelShader );
  device->setSamplerState( 1, sasSamplerState_Bilinear_Clamp, sasDeviceUnit::PixelShader );
  device->setTexture( 2, velocityBuffer, sasDeviceUnit::PixelShader );
  device->setSamplerState( 2, sasSamplerState_Bilinear_Clamp, sasDeviceUnit::PixelShader );
  device->setRenderTarget( 0, tempBuffer );
  device->setViewport( tempBuffer );

  device->setVertexShader( motionBlurShaderID );
  device->setPixelShader( motionBlurShaderID );
  device->setVertexBuffer( sasMiscResources::singletonPtr()->getFullScreenQuadVB() );  

  device->setDepthStencilTarget( NULL, false );
  device->draw( sasPrimTopology_TriStrip, 4 );

  device->setRenderTarget( 0, static_cast<sasRenderTarget*>(NULL) );
  device->setTexture( 0, static_cast<sasTexture*>(NULL), sasDeviceUnit::PixelShader );
  device->setTexture( 1, static_cast<sasTexture*>(NULL), sasDeviceUnit::PixelShader );
  device->setTexture( 2, static_cast<sasTexture*>(NULL), sasDeviceUnit::PixelShader );
  device->flushStates();

  sasRenderMng::singletonPtr()->copyRenderTarget( tempBuffer, outputBuffer );

  sasRenderTargetPool::singletonPtr()->Unlock( tempBuffer );
}