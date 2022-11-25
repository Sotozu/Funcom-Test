#include "sas_pch.h"
#include "sas_shadowmng.h"
#include "utils/sas_frustum.h"
#include "utils/sas_camera.h"
#include "render/Visibility/sas_drawlist.h"
#include "render/Visibility/sas_visibility.h"
#include "render/sas_renderlist.h"
#include "render/shaders/sas_shaderids.h"
#include "render/sas_depthbuffer.h"
#include "render/sas_rendertarget.h"
#include "render/sas_rendertargetpool.h"
#include "sas_device.h"
#include "render/sas_constantbuffer.h"
#include "render/sas_miscresources.h"
#include "utils/sas_color.h"
#include "render/debug/sas_debugmng.h"
#include "render/sas_stereo.h"
#include "render/sas_settings.h"
#include "sas_spotshadowpool.h"

struct ShadowConstants
{
  smMat44 lightViewProj[ 3 ];
  smVec4  cascadeDistances;
  smVec4  lightDirection_godrayIntensity;
  smVec4  invSMSize_invGMSize;
};

sasShadowMng::sasShadowMng()
{
  _spotShadowMapPool = sasNew sasSpotShadowPool();

  _shadowMapResolution = smVec2( 2048.f, 2048.f );

  _shadowList = sasNew sasDrawList();
  _compiledShadowList = sasNew sasRenderList();
  _shadowConstants = sasNew sasConstantBuffer( sizeof( ShadowConstants ) );
  _godraysBlurConstants = sasNew sasConstantBuffer( sizeof( smVec4 ) );

  _cascadeDistances[ 0 ] = 5.f;
  _cascadeDistances[ 1 ] = 15.f;
  _cascadeDistances[ 2 ] = 50.f;

}

sasShadowMng::~sasShadowMng()
{
  sasDelete _godraysBlurConstants;
  sasDelete _shadowConstants;
  sasDelete _compiledShadowList;
  sasDelete _shadowList;

  sasDelete _spotShadowMapPool;
}

void sasShadowMng::frameInitialize()
{
  size_t resolutionX = static_cast< size_t >( _shadowMapResolution.X );
  size_t resolutionY = static_cast< size_t >( _shadowMapResolution.Y );

  sasDepthStencilTargetDesc desc;
  desc.format = sasPixelFormat::D32;
  desc.width = resolutionX;
  desc.height = resolutionY;

  for( unsigned int i = 0; i < 3; i++ )
  {
    _shadowmap[ i ] = sasRenderTargetPool::singletonPtr()->RequestAndLock( desc );
  }

  sasRenderTargetDesc sssmDesc;
  sssmDesc.format = sasPixelFormat::R_F16;
  sssmDesc.mips = 1;
  sssmDesc.needUAV = false;
  sssmDesc.type = sasTextureType::sasTexture2D;
  sssmDesc.width = sasSettings::singletonPtr()->_frameBufferResolution.width();
  sssmDesc.height = sasSettings::singletonPtr()->_frameBufferResolution.height();
  _screenSpaceShadowMap = sasRenderTargetPool::singletonPtr()->RequestAndLock( sssmDesc );

  sasRenderTargetDesc godraysBufferDesc;
  godraysBufferDesc.format = sasPixelFormat::RGBA_F16;
  godraysBufferDesc.mips = 1;
  godraysBufferDesc.needUAV = false;
  godraysBufferDesc.type = sasTextureType::sasTexture2D;
  godraysBufferDesc.width = sasSettings::singletonPtr()->_frameBufferResolution.width() / 2;
  godraysBufferDesc.height = sasSettings::singletonPtr()->_frameBufferResolution.height() / 2;
  _godraysBuffer[ 0 ] = sasRenderTargetPool::singletonPtr()->RequestAndLock( godraysBufferDesc );
  _godraysBuffer[ 1 ] = sasRenderTargetPool::singletonPtr()->RequestAndLock( godraysBufferDesc );
  _godrayMapResolution.X = static_cast< float >( godraysBufferDesc.width );
  _godrayMapResolution.Y = static_cast< float >( godraysBufferDesc.height );

  float* consts = static_cast< float* >( _godraysBlurConstants->map() );
  consts[ 0 ] = _godraysBuffer[ 0 ]->widthFloat();
  consts[ 1 ] = _godraysBuffer[ 0 ]->heightFloat();
  consts[ 2 ] = 1.f / consts[ 0 ];
  consts[ 3 ] = 1.f / consts[ 1 ];
  _godraysBlurConstants->unmap();
}

void sasShadowMng::frameEnd()
{
  for( unsigned int i = 0; i < 3; i++ )
  {
    sasRenderTargetPool::singletonPtr()->Unlock( _shadowmap[ i ] );
  }
  sasRenderTargetPool::singletonPtr()->Unlock( _screenSpaceShadowMap );
  sasRenderTargetPool::singletonPtr()->Unlock( _godraysBuffer[ 0 ] );
  sasRenderTargetPool::singletonPtr()->Unlock( _godraysBuffer[ 1 ] );
}

void sasShadowMng::updateMatrices( sasCamera& sceneCam, const smVec3& lightDir, float vpWidth, float vpHeight )
{
  float cameraZNear = sceneCam.zNear();
  float cameraZFar = sceneCam.zFar();
  smNormalize3( lightDir, _lightDirection );

  for( unsigned int cascade_index = 0; cascade_index < 3; cascade_index++ )
  {
    sceneCam.setZPlanes( cameraZNear, _cascadeDistances[ cascade_index ] );

    if( sasStereo::singletonPtr()->enabled() )
    {
      smVec4 rightVec;
      sceneCam.getRight( rightVec );
      sceneCam.setPosition( sceneCam.position() - rightVec * ( 0.5f * sasStereo::singletonPtr()->eyeSeparationDistance() ) );
      sasFrustumPoints fpLeft( sceneCam, vpWidth, vpHeight );

      sceneCam.setPosition( sceneCam.position() + rightVec * sasStereo::singletonPtr()->eyeSeparationDistance() );
      sasFrustumPoints fpRight( sceneCam, vpWidth, vpHeight );

      sasFrustumPoints fp = sasFrustumPoints::merge( fpRight, fpLeft );

      updateMatricesForCascade( fp, sceneCam, lightDir, vpWidth, vpHeight, _lightView[ cascade_index ], _lightProj[ cascade_index ], _lightProjView[ cascade_index ] );

      //restore pos
      sceneCam.setPosition( sceneCam.position() - rightVec * ( 0.5f * sasStereo::singletonPtr()->eyeSeparationDistance() ) );
    }
    else
    {
      sasFrustumPoints fp( sceneCam, vpWidth, vpHeight );
      updateMatricesForCascade( fp, sceneCam, lightDir, vpWidth, vpHeight, _lightView[ cascade_index ], _lightProj[ cascade_index ], _lightProjView[ cascade_index ] );
    }
  }

  //update constant buffer
  smVec4 invSizes( 1.f / _shadowMapResolution.X, 1.f / _shadowMapResolution.Y, 1.f / _godrayMapResolution.X, 1.f / _godrayMapResolution.Y );
  ShadowConstants* constantData = static_cast< ShadowConstants* >( _shadowConstants->map() );
  sasASSERT( constantData != NULL );
  constantData->lightViewProj[ 0 ] = _lightProjView[ 0 ];
  constantData->lightViewProj[ 1 ] = _lightProjView[ 1 ];
  constantData->lightViewProj[ 2 ] = _lightProjView[ 2 ];
  constantData->cascadeDistances = smVec4( _cascadeDistances[ 0 ], _cascadeDistances[ 1 ], _cascadeDistances[ 2 ], 0.f );
  constantData->lightDirection_godrayIntensity = _lightDirection;
  constantData->lightDirection_godrayIntensity.W = _godraysIntensity;
  constantData->invSMSize_invGMSize = invSizes;
  _shadowConstants->unmap();

  //restore original frustum settings
  sceneCam.setZPlanes( cameraZNear, cameraZFar );
}

void sasShadowMng::updateMatricesForCascade( sasFrustumPoints& cascadeFrustum, sasCamera& sceneCam, const smVec3& lightDir, float vpWidth, float vpHeight, smMat44& view, smMat44& proj, smMat44& projView )
{
}

void sasShadowMng::renderShadowMaps( sasDevice* device, sasConstantBuffer* constantBuffer )
{
  for( unsigned int cascade_index = 0; cascade_index < 3; cascade_index++ )
  {
    sasMiscResources::singletonPtr()->updateGlobalShaderConstants( _shadowMapResolution.X, _shadowMapResolution.Y, &_lightView[ cascade_index ], &_lightProj[ cascade_index ] );

    sasFrustum frustum( _lightProjView[ cascade_index ] );
    sasVisibility::sasVisibilityQueryId visibilityResolveId;
    visibilityResolveId = sasVisibility::singleton().StartVisibilityQuery( e_CullType_Frustum, 0, &frustum, _shadowList );
    sasVisibility::singleton().GetQueryResults( visibilityResolveId );  

    device->setDepthStencilTarget( _shadowmap[ cascade_index ], false );
    device->setRenderTarget( 0, NULL );
    device->setRenderTarget( 1, NULL );
    device->setRenderTarget( 2, NULL );
    device->setDepthStencilState( sasDepthStencilState_LessEqual, 0 );
    device->setViewport( _shadowmap[ cascade_index ] );

    device->clearDepthStencilTarget( _shadowmap[ cascade_index ], sasClearFlag_DepthStencil, 1.f, 0 );
  }

  //bind shadow constants
  device->setConstantBuffer( 3, _shadowConstants, sasDeviceUnit::PixelShader );
  device->setTexture( 5, _shadowmap[ 0 ], sasDeviceUnit::PixelShader );
  device->setTexture( 6, _shadowmap[ 1 ], sasDeviceUnit::PixelShader );
  device->setTexture( 7, _shadowmap[ 2 ], sasDeviceUnit::PixelShader );
  device->setSamplerState( 7, sasSamplerState_Linear_ShadowCompare, sasDeviceUnit::PixelShader );
  device->setConstantBuffer( 3, _shadowConstants, sasDeviceUnit::ComputeShader );
  device->setTexture( 5, _shadowmap[ 0 ], sasDeviceUnit::ComputeShader );
  device->setTexture( 6, _shadowmap[ 1 ], sasDeviceUnit::ComputeShader );
  device->setTexture( 7, _shadowmap[ 2 ], sasDeviceUnit::ComputeShader );
  device->setSamplerState( 7, sasSamplerState_Linear_ShadowCompare, sasDeviceUnit::ComputeShader );
}

void sasShadowMng::computeScreenSpaceShadows( sasRenderTarget* normalBuffer, sasDepthStencilTarget* depthBuffer )
{
  INSERT_PIX_EVENT("PIXEvent: deferred shadows pass", 0xffffffff);
  sasDevice* device = sasDevice::singletonPtr();
  sasShaderID screenSpaceShadowsShader = deferredShadowsShaderID;

  if( sasStereo::singletonPtr()->enabled() )
  {
    screenSpaceShadowsShader = screenSpaceShadowsShader | stereoMask;
  }

  device->setVertexShader( screenSpaceShadowsShader );
  device->setPixelShader( screenSpaceShadowsShader );
  device->setVertexBuffer( sasMiscResources::singletonPtr()->getFullScreenQuadVB() );

  device->setTexture( 0, depthBuffer, sasDeviceUnit::PixelShader );
  device->setTexture( 1, normalBuffer, sasDeviceUnit::PixelShader );
  device->setSamplerState( 0, sasSamplerState_Point_Clamp, sasDeviceUnit::PixelShader );
  device->setRenderTarget( 0, _screenSpaceShadowMap );
  device->setRenderTarget( 1, NULL );
  device->setRenderTarget( 2, NULL );
  device->setDepthStencilTarget( NULL, false );
  device->setViewport( _screenSpaceShadowMap );
  device->draw( sasPrimTopology_TriStrip, 4 );
  device->setTexture( 0, static_cast< sasTexture* >( NULL ), sasDeviceUnit::PixelShader );
  device->setTexture( 1, static_cast< sasTexture* >( NULL ), sasDeviceUnit::PixelShader );
  device->setRenderTarget( 0, NULL );
}

void sasShadowMng::computeGodrays( sasDepthStencilTarget* depthBuffer, sasRenderTarget* volLightRT )
{
  INSERT_PIX_EVENT("PIXEvent: godrays pass", 0xffffffff);
  sasDevice* device = sasDevice::singletonPtr();

  sasShaderID godrayShader = godraysShaderID;
  if( sasStereo::singletonPtr()->enabled() )
  {
    godrayShader = godrayShader | stereoMask;
  }
  if( sasSettings::singletonPtr()->_volumetricLightingEnabled )
  {
    godrayShader = godrayShader | godraysLightScatterMask;
  }

  device->setVertexShader( godrayShader );
  device->setPixelShader( godrayShader );
  device->setVertexBuffer( sasMiscResources::singletonPtr()->getFullScreenQuadVB() );

  device->setTexture( 0, depthBuffer, sasDeviceUnit::PixelShader );
  device->setSamplerState( 0, sasSamplerState_Point_Clamp, sasDeviceUnit::PixelShader );
  device->setTexture( 1, volLightRT, sasDeviceUnit::PixelShader );
  device->setSamplerState( 1, sasSamplerState_Bilinear_Clamp, sasDeviceUnit::PixelShader );
  device->setRenderTarget( 0, _godraysBuffer[ 0 ] );
  device->setRenderTarget( 1, NULL );
  device->setRenderTarget( 2, NULL );
  device->setViewport( _godraysBuffer[ 0 ] );
  device->setDepthStencilTarget( NULL, false );
  device->draw( sasPrimTopology_TriStrip, 4 );
  device->setTexture( 0, static_cast< sasTexture* >( NULL ), sasDeviceUnit::PixelShader );
  device->setTexture( 1, static_cast< sasTexture* >( NULL ), sasDeviceUnit::PixelShader );
  device->setRenderTarget( 0, NULL );
  device->flushStates(); //flush needed due to the godray rendertarget ping ponging with the blur pass

  blurGodrays();
}

void sasShadowMng::blurGodrays()
{
  INSERT_PIX_EVENT("PIXEvent: godray blur pass", 0xffffffff);
  sasDevice* device = sasDevice::singletonPtr();
  device->setVertexShader( godraysShaderID | godraysBlurMask );
  device->setPixelShader( godraysShaderID | godraysBlurMask );
  device->setVertexBuffer( sasMiscResources::singletonPtr()->getFullScreenQuadVB() );

  device->setConstantBuffer( 1, _godraysBlurConstants, sasDeviceUnit::PixelShader );
  device->setTexture( 1, _godraysBuffer[ 0 ], sasDeviceUnit::PixelShader );
  device->setSamplerState( 1, sasSamplerState_Bilinear_Clamp, sasDeviceUnit::PixelShader );
  device->setRenderTarget( 0, _godraysBuffer[ 1 ] );
  device->setViewport( _godraysBuffer[ 1 ] );
  device->setDepthStencilTarget( NULL, false );
  device->draw( sasPrimTopology_TriStrip, 4 );
  device->setTexture( 0, static_cast< sasTexture* >( NULL ), sasDeviceUnit::PixelShader );
  device->setRenderTarget( 0, NULL );
}

void sasShadowMng::setCascadeRanges( const smVec4& ranges )
{
  _cascadeDistances[ 0 ] = ranges.X;
  _cascadeDistances[ 1 ] = ranges.Y;
  _cascadeDistances[ 2 ] = ranges.Z;
}

void sasShadowMng::clearScreenSpaceShadowMap()
{
  sasColor clearColor;
  clearColor.set( 1.f, 1.f, 1.f );
  sasDevice::singletonPtr()->clearRenderTarget( _screenSpaceShadowMap, &clearColor );
}

void sasShadowMng::clearGodrayBuffer()
{
  sasColor clearColor;
  clearColor.set( 0.f, 0.f, 0.f );
  sasDevice::singletonPtr()->clearRenderTarget( _godraysBuffer[ 1 ], &clearColor );
}

void sasShadowMng::setGodraysIntensity( float intensity )
{
  _godraysIntensity = intensity;
}