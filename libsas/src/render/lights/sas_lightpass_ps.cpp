#include "sas_pch.h"
#include "sas_lightpass_ps.h"
#include "sas_light.h"
#include "sas_lightmng.h"
#include "sas_device.h"
#include "../shaders/sas_shaderids.h"
#include "../shaders/sas_shadermng.h"
#include "../sas_rendertarget.h"
#include "../sas_indexbuffer.h"
#include "../sas_depthbuffer.h"
#include "../sas_constantbuffer.h"
#include "../sas_structuredbuffer.h"
#include "../sas_stats.h"
#include "../sas_miscresources.h"
#include "../sas_vertexbuffer.h"
#include "utils/sas_frustum.h"

static const int kMaxActiveLights = 500;

sasLightPass_ps::sasLightPass_ps()
  : _numActiveLights(0)
{
  sasVertexBuffer* unitCubeVB = sasMiscResources::singleton().getUnitCubeVB();
  _lightArrayVB = sasNew sasVertexBuffer( unitCubeVB->format(), unitCubeVB->verticesCount() * kMaxActiveLights);

  void* unitCubeDataIn = unitCubeVB->map(true);
  uint8_t* unitCubeDataOut = (uint8_t*)_lightArrayVB->map(false);
  if( unitCubeDataIn && unitCubeDataOut )
  {
    for( int i = 0; i < kMaxActiveLights; ++i )
    {
      memcpy(unitCubeDataOut, unitCubeDataIn, unitCubeVB->size());
      unitCubeDataOut += unitCubeVB->size();
    }
    unitCubeVB->unmap();
    _lightArrayVB->unmap();
  }

  _pointLightListBuffer = sasNew sasConstantBuffer( sizeof(PointlightConstantBuffer) * kMaxActiveLights);
}

sasLightPass_ps::~sasLightPass_ps()
{
  sasDelete _lightArrayVB;
  sasDelete _pointLightListBuffer;
}

void sasLightPass_ps::updateLightData( const sasFrustum& frustum )
{
  sasLightPass::updateLightData( frustum );

  sasLightMng* lightMng = sasLightMng::singletonPtr();
  PointlightConstantBuffer* constantBuffer = static_cast<PointlightConstantBuffer*>(_pointLightListBuffer->map());

  _numActiveLights = 0;

  if( constantBuffer != NULL )
  {
    for( size_t i = 0; i < lightMng->pointLightArray().size() && i < kMaxActiveLights; i++ )
    {
      constantBuffer[i].position = lightMng->pointLightArray()[i]->position();
      constantBuffer[i].radius = lightMng->pointLightArray()[i]->radius();
      constantBuffer[i].colour = lightMng->pointLightArray()[i]->colour();

      _numActiveLights++;
    }

    _pointLightListBuffer->unmap();    
  }
}

void sasLightPass_ps::performLightPass( sasRenderTarget* lightBuffer, sasRenderTarget* specBuffer, sasRenderTarget* normalBuffer, sasDepthStencilTarget* depthBuffer, 
                                          sasRenderTarget* ssaoBuffer, sasRenderTarget* /*shadowBuffer*/ )
{
  sasMiscResources& miscResources = sasMiscResources::singleton();
  sasDevice* device = sasDevice::singletonPtr();
  
  device->setSamplerState( 0, sasSamplerState_Point_Clamp, sasDeviceUnit::PixelShader );
  device->setSamplerState( 1, sasSamplerState_Point_Clamp, sasDeviceUnit::PixelShader );
  device->setTexture( 0, normalBuffer, sasDeviceUnit::PixelShader );
  device->setTexture( 1, depthBuffer, sasDeviceUnit::PixelShader );
  device->setConstantBuffer( 1, _constantBuffer, sasDeviceUnit::VertexShader );
  device->setConstantBuffer( 2, _pointLightListBuffer, sasDeviceUnit::VertexShader );
  device->setVertexShader( lightPassNoCSShaderID );
  device->setPixelShader( lightPassNoCSShaderID );
  device->setRenderTarget( 0, lightBuffer );
  device->setRenderTarget( 1, specBuffer );
  device->setVertexBuffer( _lightArrayVB );
  device->setBlendState(sasBlendState_One_One);
  device->setRasterizerState(sasRasterizerState_Solid_NoCull);
  sasLightMng* lightMng = sasLightMng::singletonPtr();

  size_t vertexCount = _numActiveLights * miscResources.getUnitCubeVB()->verticesCount();
  //printf("Setting stuff for lightpass!\n");
  device->draw( sasPrimTopology_TriList, vertexCount );
  //printf("Done with lightpass.\n");

  device->setRasterizerState(sasRasterizerState_Solid);
  device->setBlendState(sasBlendState_Null);
  device->setTexture( 0, static_cast<sasTexture*>(NULL), sasDeviceUnit::PixelShader );
  device->setTexture( 1, static_cast<sasTexture*>(NULL), sasDeviceUnit::PixelShader );
  device->setConstantBuffer( 1, NULL, sasDeviceUnit::VertexShader );
  device->setConstantBuffer( 2, NULL, sasDeviceUnit::VertexShader );
  device->setRenderTarget( 0, NULL );
  device->setRenderTarget( 1, NULL);
  device->setVertexBuffer( 0 );
  device->setVertexBuffer( 0 );
}


void sasLightPass_ps::generateVolumetricLightBuffer( sasRenderTarget* volumetricLightBuffer )
{
  sasDebugOutput( "sasLightPass_ps::generateVolumetricLightBuffer: not implemented\n" );
}