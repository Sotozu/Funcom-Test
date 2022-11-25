#include "sas_pch.h"
#include "sas_compositePass.h"
#include "sas_device.h"
#include "../shaders/sas_shaderids.h"
#include "../shaders/sas_shadermng.h"
#include "../sas_rendertarget.h"
#include "../sas_depthbuffer.h"
#include "../sas_constantbuffer.h"
#include "../sas_rendertargetpool.h"
#include "../sas_rendermng.h"
#include "../sas_miscresources.h"
#include "../shadows/sas_shadowmng.h"


struct sasCompositeConstants
{
  smVec4 godraysResInvRes;
};

sasCompositePass::sasCompositePass()
{
  _cb = sasNew sasConstantBuffer( sizeof( smVec4 ) );
}

sasCompositePass::~sasCompositePass()
{
  sasDelete _cb;
}

void sasCompositePass::applyCompositePass( sasRenderTarget* diffuseBuffer, sasRenderTarget* lightBuffer, sasRenderTarget* specBuffer, sasRenderTarget* godraysBuffer, 
                                          sasDepthStencilTarget* depthBuffer, sasRenderTarget* output )
{
  sasCompositeConstants* consts = static_cast< sasCompositeConstants* >( _cb->map() );
  consts->godraysResInvRes = smVec4( godraysBuffer->widthFloat(), godraysBuffer->heightFloat(), 1.f / godraysBuffer->widthFloat(), 1.f / godraysBuffer->heightFloat() );
  _cb->unmap();

  sasDevice* d = sasDevice::singletonPtr();
  d->setVertexShader( compositePassShaderID );
  d->setPixelShader( compositePassShaderID );
  d->setVertexBuffer( sasMiscResources::singletonPtr()->getFullScreenQuadVB() );
  d->setTexture( 0, diffuseBuffer, sasDeviceUnit::PixelShader );
  d->setSamplerState( 0, sasSamplerState_Point_Clamp, sasDeviceUnit::PixelShader );
  d->setTexture( 1, lightBuffer, sasDeviceUnit::PixelShader );
  d->setSamplerState( 1, sasSamplerState_Point_Clamp, sasDeviceUnit::PixelShader );
  d->setTexture( 2, specBuffer, sasDeviceUnit::PixelShader );
  d->setSamplerState( 2, sasSamplerState_Point_Clamp, sasDeviceUnit::PixelShader );
  d->setTexture( 3, depthBuffer, sasDeviceUnit::PixelShader );
  d->setSamplerState( 3, sasSamplerState_Point_Clamp, sasDeviceUnit::PixelShader );
  d->setTexture( 4, godraysBuffer, sasDeviceUnit::PixelShader );
  d->setSamplerState( 4, sasSamplerState_Bilinear_Clamp, sasDeviceUnit::PixelShader );
  d->setRenderTarget( 0, output );
  d->setViewport( output );
  d->setDepthStencilTarget( NULL, false );
  d->setConstantBuffer( 1, _cb, sasDeviceUnit::PixelShader );
  d->draw( sasPrimTopology_TriStrip, 4 );

  d->setTexture( 3, static_cast< sasRenderTarget* >( nullptr ), sasDeviceUnit::PixelShader );
  d->flushStates();  
}

