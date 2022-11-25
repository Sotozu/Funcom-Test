#include "sas_pch.h"
#include "sas_lightpass_cs.h"
#include "sas_light.h"
#include "sas_lightmng.h"
#include "sas_device.h"
#include "../shaders/sas_shaderids.h"
#include "../shaders/sas_shadermng.h"
#include "../sas_rendertarget.h"
#include "../sas_depthbuffer.h"
#include "../sas_constantbuffer.h"
#include "../sas_structuredbuffer.h"
#include "../sas_stats.h"
#include "utils/sas_frustum.h"
#include "render/sas_stereo.h"

struct pointLightData
{
  smVec4 position;
  smVec4 colour_radius;
};

struct spotLightData
{
  smVec4 position_innerAngle;
  smVec4 colour_radius;
  smVec4 dir_OuterAngle;
  smVec4 smIndexPadding;
};

sasLightPass_cs::sasLightPass_cs()
{
  _pointLightArrayBuffer = sasNew sasStructuredBuffer( kMaxPointLights * sizeof( pointLightData ), sizeof( pointLightData ), true, false );
  _spotLightArrayBuffer = sasNew sasStructuredBuffer( kMaxSpotLights * sizeof( spotLightData ), sizeof( spotLightData ), true, false );
}

sasLightPass_cs::~sasLightPass_cs()
{
  sasDelete _spotLightArrayBuffer;
  sasDelete _pointLightArrayBuffer; 
}

void sasLightPass_cs::updateLightData( const sasFrustum& frustum )
{
  sasLightPass::updateLightData( frustum );

  sasLightMng* lightMng = sasLightMng::singletonPtr();

  //point lights
  pointLightData* lightData = static_cast< pointLightData* >( _pointLightArrayBuffer->map() );
  for( unsigned int i = 0; i < lightMng->culledPointLightArray().size(); i++ )
  {
    lightData->position = lightMng->culledPointLightArray()[i]->position();
    lightData->colour_radius = lightMng->culledPointLightArray()[i]->colour();
    lightData->colour_radius.W = lightMng->culledPointLightArray()[i]->radius();
    lightData++;
  }
  //Null light
  lightData->position = smVec4(0.f, 0.f, 0.f, 0.f);
  lightData->colour_radius = smVec4(0.f, 0.f, 0.f, 0.f);
  _pointLightArrayBuffer->unmap();

  //spot lights
  spotLightData* spotLight = static_cast< spotLightData* >( _spotLightArrayBuffer->map() );
  for( unsigned int i = 0; i < lightMng->culledSpotLightArray().size(); i++ )
  {
    spotLight->position_innerAngle = lightMng->culledSpotLightArray()[i]->position();
    spotLight->position_innerAngle.W = cosf( smDeg2Rad( lightMng->culledSpotLightArray()[i]->innerAngle() ) );
    spotLight->colour_radius = lightMng->culledSpotLightArray()[i]->colour();
    spotLight->colour_radius.W = lightMng->culledSpotLightArray()[i]->radius();
    spotLight->dir_OuterAngle = lightMng->culledSpotLightArray()[i]->direction();
    spotLight->dir_OuterAngle.W = cosf( smDeg2Rad( lightMng->culledSpotLightArray()[i]->outerAngle() ) );
    spotLight->smIndexPadding = smVec4( static_cast< float >( lightMng->culledSpotLightArray()[i]->shadowMapIndex() ), 0.f, 0.f, 0.f );
    spotLight++;
  }
  //Null light
  spotLight->position_innerAngle = smVec4(0.f, 0.f, 0.f, 0.f);
  spotLight->colour_radius = smVec4(0.f, 0.f, 0.f, 0.f);
  spotLight->dir_OuterAngle = smVec4(0.f, 0.f, 0.f, 0.f);
  spotLight->smIndexPadding = smVec4( -1.f, 0.f, 0.f, 0.f );
  _spotLightArrayBuffer->unmap();
}

void sasLightPass_cs::performLightPass( sasRenderTarget* lightBuffer, sasRenderTarget* specBuffer, sasRenderTarget* normalBuffer, sasDepthStencilTarget* depthBuffer, 
                                          sasRenderTarget* ssaoBuffer, sasRenderTarget* shadowBuffer )
{
  sasDevice* device = sasDevice::singletonPtr();

  if( !sasStereo::singleton().enabled() )
  { //non stereo lighting

    device->setConstantBuffer( 1, _constantBuffer, sasDeviceUnit::ComputeShader );
    device->setTexture( 2, normalBuffer, sasDeviceUnit::ComputeShader );
    device->setTexture( 1, depthBuffer, sasDeviceUnit::ComputeShader );
    device->setTexture( 3, ssaoBuffer, sasDeviceUnit::ComputeShader );
    device->setTexture( 4, shadowBuffer, sasDeviceUnit::ComputeShader );
    device->setSamplerState( 3, sasSamplerState_Bilinear_Clamp, sasDeviceUnit::ComputeShader );
    device->setStructuredBuffer( 0, _pointLightArrayBuffer, sasDeviceUnit::ComputeShader );
    device->setStructuredBuffer( 5, _spotLightArrayBuffer, sasDeviceUnit::ComputeShader );
    device->setComputeShader( lightPassShaderID );
    device->setUAV( 0, lightBuffer, sasDeviceUnit::ComputeShader );
    device->setUAV( 1, specBuffer, sasDeviceUnit::ComputeShader );

    size_t nGroupX = (lightBuffer->width() + 15 ) / 16;
    size_t nGroupY = (lightBuffer->height() + 15 ) / 16;
    device->dispatch( nGroupX, nGroupY, 1 );

  }
  else //Stereo lighting
  {

    /*
      Left eye
    */
    device->setConstantBuffer( 1, _constantBuffer, sasDeviceUnit::ComputeShader );
    device->setTexture( 2, normalBuffer, sasDeviceUnit::ComputeShader );
    device->setTexture( 1, depthBuffer, sasDeviceUnit::ComputeShader );
    device->setTexture( 3, ssaoBuffer, sasDeviceUnit::ComputeShader );
    device->setTexture( 4, shadowBuffer, sasDeviceUnit::ComputeShader );
    device->setSamplerState( 3, sasSamplerState_Bilinear_Clamp, sasDeviceUnit::ComputeShader );
    device->setStructuredBuffer( 0, _pointLightArrayBuffer, sasDeviceUnit::ComputeShader );
    device->setStructuredBuffer( 5, _spotLightArrayBuffer, sasDeviceUnit::ComputeShader );
    device->setComputeShader( lightPassShaderID | stereoMask | lightPassStereoLeftEyeMask );
    device->setUAV( 0, lightBuffer, sasDeviceUnit::ComputeShader );
    device->setUAV( 1, specBuffer, sasDeviceUnit::ComputeShader );

    size_t halfWidth = lightBuffer->width() / 2;
    size_t nGroupX = ( halfWidth + 15 ) / 16;
    size_t nGroupY = ( lightBuffer->height() + 15 ) / 16;
    device->dispatch( nGroupX, nGroupY, 1 );

    /*
      Right eye
    */
    device->setComputeShader( lightPassShaderID | stereoMask | lightPassStereoRightEyeMask );
    device->dispatch( nGroupX, nGroupY, 1 );
  }

  device->setUAV( 0, static_cast<sasRenderTarget*>(NULL), sasDeviceUnit::ComputeShader );
  device->setUAV( 1, static_cast<sasRenderTarget*>(NULL), sasDeviceUnit::ComputeShader );
  device->setUAV( 2, static_cast<sasRenderTarget*>(NULL), sasDeviceUnit::ComputeShader );
  device->setTexture( 0, static_cast<sasTexture*>(NULL), sasDeviceUnit::ComputeShader );
  device->setTexture( 1, static_cast<sasTexture*>(NULL), sasDeviceUnit::ComputeShader );
  device->setTexture( 3, static_cast<sasTexture*>(NULL), sasDeviceUnit::ComputeShader );
  device->setTexture( 4, static_cast<sasTexture*>(NULL), sasDeviceUnit::ComputeShader );
  device->setTexture( 5, static_cast<sasTexture*>(NULL), sasDeviceUnit::ComputeShader );
}

void sasLightPass_cs::generateVolumetricLightBuffer( sasRenderTarget* volumetricLightBuffer )
{
  sasDevice* device = sasDevice::singletonPtr();
  device->setConstantBuffer( 1, _constantBuffer, sasDeviceUnit::ComputeShader );
  device->setStructuredBuffer( 0, _pointLightArrayBuffer, sasDeviceUnit::ComputeShader );
  device->setStructuredBuffer( 1, _spotLightArrayBuffer, sasDeviceUnit::ComputeShader );
  device->setTexture( 2, device->depthBuffer(), sasDeviceUnit::ComputeShader );
  device->setSamplerState( 0, sasSamplerState_Bilinear_Clamp, sasDeviceUnit::ComputeShader );
  device->setComputeShader( genVolLightPassShaderID );
  device->setUAV( 0, volumetricLightBuffer, sasDeviceUnit::ComputeShader );

  size_t nGroupX = (volumetricLightBuffer->width() + 7 ) / 8;
  size_t nGroupY = (volumetricLightBuffer->height() + 7 ) / 8;
  size_t nGroupZ = (volumetricLightBuffer->depth() + 7 ) / 8;
  device->dispatch( nGroupX, nGroupY, nGroupZ );

  device->setUAV( 0, static_cast<sasRenderTarget*>(NULL), sasDeviceUnit::ComputeShader );
  device->setTexture( 0, static_cast<sasTexture*>(NULL), sasDeviceUnit::ComputeShader );
  device->setTexture( 1, static_cast<sasTexture*>(NULL), sasDeviceUnit::ComputeShader );
  device->setTexture( 2, static_cast<sasTexture*>(NULL), sasDeviceUnit::ComputeShader );
  device->setConstantBuffer( 1, NULL, sasDeviceUnit::ComputeShader );
}



smVec4 clipSpaceToViewSpace( const smVec4& p, const smMat44& invProj )
{
  smVec4 r;
  smMul( invProj, p, r );
  return r / r.W;
}


smVec4 computePlaneEquationWithOriginZero( const smVec4& b, const smVec4& c )
{
  smVec4 n;
  // normalize(cross( b.xyz-a.xyz, c.xyz-a.xyz )), except we know "a" is the origin
  smNormalize3( smCross( b, c ), n );
  // -(n dot a), except we know "a" is the origin
  n.W = 0;
  return n;
}

void sasLightPass_cs::generateLightListPerTile( uint32_t tileSize, float resX, float resY, const sasFrustum& frustum, const smMat44& viewProj, const smMat44& invProj )
{
  uint32_t numTilesX = static_cast< uint32_t >( floorf( ( resX + tileSize - 1 ) / tileSize ) );
  uint32_t numTilesY = static_cast< uint32_t >( floorf( ( resY + tileSize - 1 ) / tileSize ) );

  for( uint32_t tileIdxX = 0; tileIdxX < numTilesX; tileIdxX++ )
  {
    for( uint32_t tileIdxY = 0; tileIdxY < numTilesY; tileIdxY++ )
    {
      uint32_t tileIndex = tileIdxY * numTilesX + tileIdxX;

      uint32_t pxm = tileSize * tileIdxX;
      uint32_t pym = tileSize * tileIdxY;
      uint32_t pxp = tileSize * ( tileIdxX + 1);
      uint32_t pyp = tileSize * ( tileIdxY + 1 );

      // four corners of the tile, clockwise from top-left
      smVec4 frustumPts[ 4 ];
      frustumPts[ 0 ] = clipSpaceToViewSpace( smVec4( pxm / resX * 2.f - 1.f, ( resY - pym ) / resY * 2.f - 1.f, 1.f, 1.f ), invProj );
      frustumPts[ 1 ] = clipSpaceToViewSpace( smVec4( pxp / resX * 2.f - 1.f, ( resY - pym ) / resY * 2.f - 1.f, 1.f, 1.f ), invProj );
      frustumPts[ 2 ] = clipSpaceToViewSpace( smVec4( pxp / resX * 2.f - 1.f, ( resY - pyp ) / resY * 2.f - 1.f, 1.f, 1.f ), invProj );
      frustumPts[ 3 ] = clipSpaceToViewSpace( smVec4( pxm / resX * 2.f - 1.f, ( resY - pyp ) / resY * 2.f - 1.f, 1.f, 1.f ), invProj );

      smVec4 frustumEqn[ 4 ];
      frustumEqn[ 0 ] = computePlaneEquationWithOriginZero( frustumPts[ 0 ], frustumPts[ 1 ] );
      frustumEqn[ 1 ] = computePlaneEquationWithOriginZero( frustumPts[ 1 ], frustumPts[ 2 ] );
      frustumEqn[ 2 ] = computePlaneEquationWithOriginZero( frustumPts[ 2 ], frustumPts[ 3 ] );
      frustumEqn[ 3 ] = computePlaneEquationWithOriginZero( frustumPts[ 3 ], frustumPts[ 0 ] );

      for( uint32_t lIndex = 0; lIndex < /*NUMLIGHTS*/0; lIndex++ )
      {
        smVec4 lightPosVS;
        float lightRadius = 0.f;
        if( smDot( lightPosVS, frustumEqn[ 0 ] ) < lightRadius 
         && smDot( lightPosVS, frustumEqn[ 1 ] ) < lightRadius 
         && smDot( lightPosVS, frustumEqn[ 2 ] ) < lightRadius 
         && smDot( lightPosVS, frustumEqn[ 3 ] ) < lightRadius )
        {
          //add light to list, only check for z bounds in CS
        }
      }
      
    } //tileY loop 
  } //tileX loop

}
