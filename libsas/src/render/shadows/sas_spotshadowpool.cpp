#include "sas_pch.h"
#include "sas_spotshadowpool.h"
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
#include "render/lights/sas_light.h"
#include "render/debug/sas_debugmng.h"
#include "utils/sas_culling.h"

struct SpotShadowConstants
{
  smMat44 lightViewProj[ 4 ];
  smVec4  invSMSize[ 4 ];
};

sasSpotShadowPool::sasSpotShadowPool()
{
  for( uint32_t i = 0; i < kMaxShadowMapEntries; i++ )
  {
    _spotShadowMapEntries[ i ] = sasNew sasSpotShadowPoolEntry( 768, 768 );
  }

  _shadowConstants = sasNew sasConstantBuffer( sizeof( SpotShadowConstants ) );
}

sasSpotShadowPool::~sasSpotShadowPool()
{
  for( uint32_t i = 0; i < kMaxShadowMapEntries; i++ )
  {
    sasDelete _spotShadowMapEntries[ i ];
    _spotShadowMapEntries[ i ] = NULL;
  }

  sasDelete _shadowConstants;
}

void sasSpotShadowPool::preRenderStep( const sasCamera& cam, const sasFrustum& frustum )
{
  struct visibleLightData
  {
    sasSpotLight* light;
    float         distFromCam;
  };

  visibleLightData visLights[ kMaxShadowMapEntries ];
  uint32_t numVisibleLights = 0;

  //search for most relevant shadow casting lights
  for( size_t i = 0; i < _entryLinks.size(); i++ )
  {
    sasSpotLight* l = _entryLinks[ i ].first;
    //unbind shadow map from light, it might change, will be reset when new index is available
    l->setShadowMapIndex( -1 );

    smVec4 lightVolume( l->position() );
    lightVolume.W = l->radius();
    if( sasIntersects( &frustum, &lightVolume ) )
    {
      visibleLightData vld;
      vld.light = l;
      vld.distFromCam = smDot3( l->position(), cam.position() );

      if( numVisibleLights < kMaxShadowMapEntries )
      {
        visLights[ numVisibleLights ] = vld;
        numVisibleLights++;
      }
      else
      {
        //find most distant visible shadow casting light
        int furthestDistIndex = 0;
        for( size_t j = 1; j < kMaxShadowMapEntries; j++ )
        {
          if( visLights[ j ].distFromCam > visLights[ furthestDistIndex ].distFromCam )
          {
            furthestDistIndex = j;
          }
        }

        //replace if current light is closer than furthest light in list
        if( vld.distFromCam < visLights[ furthestDistIndex ].distFromCam )
          visLights[ furthestDistIndex ] = vld;
      }
    }
  }

  //set lights to entries
  for( uint32_t i = 0; i < numVisibleLights; i++ )
  {
    _spotShadowMapEntries[ i ]->setLight( visLights[ i ].light );
    _spotShadowMapEntries[ i ]->setupRenderPass();
    visLights[ i ].light->setShadowMapIndex( i );
  }
  for( int i = numVisibleLights; i < kMaxShadowMapEntries; i++ )
  {
    _spotShadowMapEntries[ i ]->setLight( NULL );
  }

  //update spot shadow constant block
  SpotShadowConstants* constData = static_cast< SpotShadowConstants* >( _shadowConstants->map() );
  for( unsigned int i = 0; i < 4; i++ )
  {
    constData->lightViewProj[ i ] = _spotShadowMapEntries[ i ]->projViewMtx();
    constData->invSMSize[ i ] = smVec4( 1.f / _spotShadowMapEntries[ i ]->resolution().X, 1.f / _spotShadowMapEntries[ i ]->resolution().Y, 0.f, 0.f );
  }
  _shadowConstants->unmap();
}

void sasSpotShadowPool::updateShadowMaps( sasDevice* device, sasConstantBuffer* constantBuffer )
{
  for( int i = 0; i < kMaxShadowMapEntries; i++ )
  {
    if( _spotShadowMapEntries[ i ]->_isInUse() )
      _spotShadowMapEntries[ i ]->renderShadowMap( device, constantBuffer );
  }

  //bind spot shadow resources to device context
  device->setConstantBuffer( 5, _shadowConstants, sasDeviceUnit::ComputeShader );
  for( unsigned int i = 0; i < 4; i++ )
  {
    device->setTexture( 8 + i, _spotShadowMapEntries[ i ]->shadowMap(), sasDeviceUnit::ComputeShader );
  }
}

void sasSpotShadowPool::registerSpotLight( sasSpotLight* light )
{
  if( findSpotLightEntryIndex( light ) >= 0 )
  {
    sasDebugOutput( "sasSpotShadowPool::registerSpotLight - spot light is already registered into the shadowmap pool...\n" );
    return;
  }

  SMPoolEntry e;
  e.first = light;
  e.second = -1;
  _entryLinks.push_back( e );
}

void sasSpotShadowPool::unregisterSpotLight( sasSpotLight* light )
{
  int entryIndex = findSpotLightEntryIndex( light );
  if( entryIndex >= 0 )
  {
    _entryLinks[ entryIndex ].first->setShadowMapIndex( -1 );
    _entryLinks[ entryIndex ] = _entryLinks[ _entryLinks.size() - 1 ];
    _entryLinks.pop_back();
  }
  else
  {
    sasDebugOutput( "sasSpotShadowPool::unregisterSpotLight - spot light has never been registered into the shadowmap pool...\n" );
    return;
  }
}

int sasSpotShadowPool::findSpotLightEntryIndex( sasSpotLight* light ) const
{
  for( size_t i = 0; i < _entryLinks.size(); i++ )
  {
    if( _entryLinks[ i ].first == light )
    {
      sasASSERT( _entryLinks[ i ].second < kMaxShadowMapEntries );
      return i;//_entryLinks[ i ].second;
    }
  }
  return -1;
}

void sasSpotShadowPool::stepPostFrame()
{
  for( int i = 0; i < kMaxShadowMapEntries; i++ )
  {
      _spotShadowMapEntries[ i ]->stepPostFrame();
  }
}


/*

 Spotlight shadowmap pool entry

*/

sasSpotShadowPoolEntry::sasSpotShadowPoolEntry( size_t resolutionX, size_t resolutionY )
  : _shadowMapResolution( static_cast< float >( resolutionX ), static_cast< float >( resolutionY ) )
  , _shadowMap( NULL )
  , _light( NULL )
  , _shadowConstants( NULL )
  , _needUpdate( false )
{
  _compiledShadowList = sasNew sasRenderList();
  _shadowList = sasNew sasDrawList();
  _shadowConstants = sasNew sasConstantBuffer( sizeof( SpotShadowConstants ) );
}

sasSpotShadowPoolEntry::~sasSpotShadowPoolEntry()
{
  if( _shadowMap )
    sasRenderTargetPool::singletonPtr()->Unlock( _shadowMap );

  sasDelete _shadowConstants;
  sasDelete _shadowList;
  sasDelete _compiledShadowList;
}

void sasSpotShadowPoolEntry::setLight( sasSpotLight* light )
{
  if( light != _light )
  {
    _light = light;
    _needUpdate = true;
  }
}

void sasSpotShadowPoolEntry::setupRenderPass()
{
  if( !_needUpdate || ( _light == NULL ) )
    return;

  smVec4 pos = _light->position();
  smVec4 target = pos + _light->direction();
  if( ( _light->direction().X == 0.f ) && ( _light->direction().Z == 0 ) ) //prevent up and direction vectors from being coplanar
  {
    target += smVec4( 0.1f, 0.f, 0.f, 0.f );
  }
  float farZ = _light->radius();
  
  sasCamera camera;
  camera.setPosition( pos );
  camera.setZPlanes( 0.05f, farZ );
  camera.setFOV( _light->outerAngle() );
  camera.lookAt( target, smVec4( 0.f, 1.f, 0.f, 0.f ) );
  camera.getProjMatrix( _shadowMapResolution.X, _shadowMapResolution.Y, _projMtx );
  camera.getViewMatrix( _viewMtx );
  smMul( _projMtx, _viewMtx, _projViewMtx );

  sasFrustum frustum( _projViewMtx );
  sasVisibility::sasVisibilityQueryId visibilityResolveId;
  visibilityResolveId = sasVisibility::singleton().StartVisibilityQuery( e_CullType_Frustum, 0, &frustum, _shadowList );
  sasVisibility::singleton().GetQueryResults( visibilityResolveId );
}

void sasSpotShadowPoolEntry::renderShadowMap( sasDevice* device, sasConstantBuffer* constantBuffer )
{
  sasDepthStencilTargetDesc dstDesc;
  dstDesc.format = sasPixelFormat::D32;
  dstDesc.width = static_cast< size_t >( _shadowMapResolution.X );
  dstDesc.height = static_cast< size_t >( _shadowMapResolution.Y );
  dstDesc.type = sasTextureType::sasTexture2D;

  _shadowMap = sasRenderTargetPool::singletonPtr()->RequestAndLock( dstDesc );
  sasASSERT( _shadowMap );

  sasMiscResources::singletonPtr()->updateGlobalShaderConstants( _shadowMapResolution.X, _shadowMapResolution.Y, &_viewMtx, &_projMtx );

  device->setDepthStencilTarget( _shadowMap, false );
  device->setRenderTarget( 0, NULL );
  device->setRenderTarget( 1, NULL );
  device->setRenderTarget( 2, NULL );
  device->setDepthStencilState( sasDepthStencilState_LessEqual, 0 );
  device->setViewport( _shadowMap );

  device->clearDepthStencilTarget( _shadowMap, sasClearFlag_DepthStencil, 1.f, 0 );

  //Render draw list
  {
    INSERT_PIX_EVENT("PIXEvent: spotlight shadowmap pass", 0xffffffff);
    if( !sasDebugMng::singletonPtr()->newStyleRenderLists() )
      sasRenderList::renderDrawListImmediate( _shadowList, sasRenderList_ShadowPass, &_projViewMtx, constantBuffer );
    else
    {
      _compiledShadowList->processList( _shadowList, sasRenderList_ShadowPass, &_projViewMtx, _light->position() );
      _compiledShadowList->renderList( constantBuffer );
    }
  }
}

void sasSpotShadowPoolEntry::stepPostFrame()
{
  if( _shadowMap )
  {
    sasRenderTargetPool::singletonPtr()->Unlock( _shadowMap );
    _shadowMap = NULL;
  }
}
