#include "sas_pch.h"
#include "sas_rendermng.h"
#include "utils/sas_color.h"
#include "sas_instance.h"
#include "sas_geometry.h"
#include "sas_mesh.h"
#include "gui/sas_gui.h"
#include "sas_indexbuffer.h"
#include "sas_vertexbuffer.h"
#include "sas_device.h"
#include "shaders/sas_shadermng.h"
#include "sas_main.h"
#include "utils/sas_cameramng.h"
#include "utils/sas_camera.h"
#include "sas_constantbuffer.h"
#include "sasMaths.h"
#include "visibility/sas_visibility.h"
#include "utils/sas_frustum.h"
#include "sas_stats.h"
#include "sas_material.h"
#include "sas_rendertargetpool.h"
#include "sas_miscresources.h"
#include "shaders/sas_shaderids.h"
#include "sas_renderlist.h"
#include "lights/sas_lightpass_cs.h"
#include "lights/sas_lightpass_ps.h"
#include "lights/sas_volumetricLight.h"
#include "sas_rendertarget.h"
#include "visibility/sas_drawlist.h"
#include "debug/sas_debugmng.h"
#include "debug/sas_debugrender.h"
#include "sas_settings.h"
#include "motionBlur/sas_motionBlur.h"
#include "shadows/sas_shadowmng.h"
#include "shadows/sas_spotshadowpool.h"
#include "resource/sas_resourcemng.h"
#include "postAA/sas_postaa.h"
#include "resource/sas_splashscreen.h"
#include "lights/sas_lightmng.h"
#include "editor/sas_editor.h"
#include "profiling/sas_gpuprofiler.h"
#include "profiling/sas_cpuprofiler.h"
#include "sas_stereo.h"
#include "compositePass/sas_compositePass.h"
#include "utils/sas_timer.h"

sasRenderMng::sasRenderMng( const sasConfig& config )
  : _shaderMng(0)
  , _sasDevice(0)
  , _constantBuffer(0)
  , _visibilityMng(0)
  , _mainDrawList(0)
  , _stats(0)
  , _rtPool(0)
  , _miscResources(0)
  , _gui(0)
  , _uiDisabled( false )
{ 
  _sasDevice = sasNew sasDevice( config );  
  _shaderMng = sasNew sasShaderMng( config.shaderFolder );     
  _constantBuffer = sasNew sasConstantBuffer( sizeof( sasPerInstanceConstants ) );
  _visibilityMng = sasNew sasVisibility();
  _rtPool = sasNew sasRenderTargetPool();
  _miscResources = sasNew sasMiscResources();
  _mainDrawList = sasNew sasDrawList();
  _mainRenderList = sasNew sasRenderList();

  if( _sasDevice->queryCapability( sasCapability_ComputeShaders ) )
  {
    _lightPass = sasNew sasLightPass_cs();    
  }
  else
  {
    _lightPass = sasNew sasLightPass_ps();    
  }

  _compositePass = sasNew sasCompositePass;
  _renderSettings = sasNew sasSettings( _sasDevice );
  _gui = sasNew sasGui();
  _stats = sasNew sasRenderStats();
  _debugMng = sasNew sasDebugMng();
  _debugRender = sasNew sasDebugRender();
  _shadowMng = sasNew sasShadowMng();
  _postAA = sasNew sasPostAA();
  _editor = sasNew sasEditor();
  _gpuProfiler = sasNew sasGpuProfiler();
  _stereo = sasNew sasStereo( config );
  
  _shaderMng->initializeShaderIds();
}

sasRenderMng::~sasRenderMng()
{
  // reverse order
  sasDelete _stereo;
  sasDelete _gpuProfiler;
  sasDelete _editor;
  sasDelete _postAA;
  sasDelete _shadowMng;
  sasDelete _debugRender;
  sasDelete _debugMng;
  sasDelete _stats;
  sasDelete _gui;
  sasDelete _renderSettings;
  sasDelete _compositePass;
  sasDelete _lightPass;
  sasDelete _mainRenderList;
  sasDelete _mainDrawList;
  sasDelete _miscResources;
  sasDelete _rtPool;
  sasDelete _visibilityMng;
  sasDelete _constantBuffer;    
  sasDelete _shaderMng;    
  sasDelete _sasDevice;    
}

void sasRenderMng::update()
{
  _stats->reset();

  adjustBackBufferSize();

  sasResourceMng::singleton().stepPreRender();
  _sasDevice->setDefaultState();

  sasCamera* cam = sasCameraMng::singleton().getCurrentCamera();
  _stereo->stepPreRender( *cam );

  renderFrame(); 

  _editor->step();

  updatePostFrame();
}

void sasRenderMng::updatePostFrame()
{
  sasSpotShadowPool::singletonPtr()->stepPostFrame();


  sasInstance::Iterator instanceIt  =   sasInstance::begin();
  sasInstance::Iterator last        =   sasInstance::end();

  while( instanceIt != last )
  {
    ( *instanceIt )->updateInstancePostFrame();
    instanceIt++;
  }
}

void sasRenderMng::adjustBackBufferSize()
{
  if( _sasDevice->resizeBackBuffer() )
  {
    _rtPool->Clear();
    sasGui::singleton().resizeWindow();
  }
}

void sasRenderMng::renderFrame()
{
  //Setup shadows
  _shadowMng->frameInitialize();

  //Setup GBuffer
  sasRenderTargetDesc rtDesc;
  rtDesc.format = sasPixelFormat::RGBA_U8;
  rtDesc.mips = 1;
  rtDesc.needUAV = false;
  rtDesc.width = sasSettings::singletonPtr()->_frameBufferResolution.width();
  rtDesc.height = sasSettings::singletonPtr()->_frameBufferResolution.height();
  sasRenderTarget* normalRT = _rtPool->RequestAndLock( rtDesc );
  sasRenderTarget* diffuseRT = _rtPool->RequestAndLock( rtDesc );
  rtDesc.format = sasPixelFormat::RG_F16;
  sasRenderTarget* velocityRT = _rtPool->RequestAndLock( rtDesc );
  rtDesc.format = sasPixelFormat::RGBA_U8;
  rtDesc.needUAV = true;
  sasRenderTarget* ssaoRT = _rtPool->RequestAndLock( rtDesc );
  rtDesc.format = sasPixelFormat::RGBA_F16;
  sasRenderTarget* lightRT = _rtPool->RequestAndLock( rtDesc );
  sasRenderTarget* specRT = _rtPool->RequestAndLock( rtDesc );

  _sasDevice->setRenderTarget( 0, normalRT );
  _sasDevice->setRenderTarget( 1, diffuseRT );
  _sasDevice->setRenderTarget( 2, velocityRT );
  _sasDevice->setDepthStencilTarget( _sasDevice->depthBuffer(), false );
  _sasDevice->setViewport( normalRT );

  sasColor clearColor;
  clearColor.set( 0.f, 0.f, 0.f );
  _sasDevice->clearRenderTarget( normalRT, &clearColor );
  _sasDevice->clearRenderTarget( diffuseRT, &clearColor );
  _sasDevice->clearRenderTarget( lightRT, &clearColor );
  _sasDevice->clearRenderTarget( specRT, &clearColor );
  _sasDevice->clearRenderTarget( velocityRT, &clearColor );
  _sasDevice->setDepthStencilState( sasDepthStencilState_LessEqual, 0 );
  _sasDevice->setRasterizerState( sasRasterizerState_Solid );

  //runtime cubemap
  sasRenderTargetDesc cubeDesc;
  cubeDesc.format = sasPixelFormat::BGRA_U8;
  cubeDesc.height = 128;
  cubeDesc.width = 128;
  cubeDesc.mips = 1;
  cubeDesc.needUAV = false;
  cubeDesc.type = sasTextureType::sasTextureCube;
  sasRenderTarget* cubeMapRT = _rtPool->RequestAndLock( cubeDesc );
  
  //volumetric light buffer
  sasRenderTargetDesc volLightBufDesc;
  volLightBufDesc.format = sasPixelFormat::RGBA_F16;
  volLightBufDesc.height = 96;
  volLightBufDesc.width = 128;
  volLightBufDesc.depth = 64;
  volLightBufDesc.mips = 1;
  volLightBufDesc.needUAV = true;
  volLightBufDesc.type = sasTextureType::sasTexture3D;
  sasRenderTarget* volLightRT = _rtPool->RequestAndLock( volLightBufDesc );

  static std::string frameTimeGpuEvenName = "TotalFrameTime";
  _gpuProfiler->startGpuTimer( frameTimeGpuEvenName );

  smMat44 matView;
  smMat44 matProj;
  sasCamera* camera = sasCameraMng::singleton().getCurrentCamera();
  if( camera )
  {
    //Setup global constants
    _miscResources->updateGlobalShaderConstants( sasSettings::singletonPtr()->_frameBufferResolution.widthFloat(), sasSettings::singletonPtr()->_frameBufferResolution.heightFloat() );

    camera->getViewMatrix( matView );
    float bbWidth = sasSettings::singletonPtr()->_frameBufferResolution.widthFloat();
    float bbHeight = sasSettings::singletonPtr()->_frameBufferResolution.heightFloat();
    if( _stereo->enabled() )
      bbWidth *= 0.5f;

    camera->getProjMatrix( bbWidth, bbHeight, matProj );

    smMat44 matProjView;
    smMul( matProj, matView, matProjView );
    sasFrustum frustum( matProjView );

    if( _renderSettings->_shadowsEnabled )
    {
      //directional shadows
      {
        INSERT_GPU_TIMER( ShadowPass, "ShadowPass" );
        _shadowMng->updateMatrices( *camera, sasLightMng::singletonPtr()->sunDir(), bbWidth, bbHeight );
        _shadowMng->renderShadowMaps( _sasDevice, _constantBuffer );
      }

      //shadow map pools update
      {
        INSERT_GPU_TIMER( SpotShadowPass, "SpotShadowPass" );
        sasSpotShadowPool::singletonPtr()->preRenderStep( *camera, frustum );
        sasSpotShadowPool::singletonPtr()->updateShadowMaps( _sasDevice, _constantBuffer );
      }
    }

    //reset global shader constants for main camera view
    _miscResources->updateGlobalShaderConstants( sasSettings::singletonPtr()->_frameBufferResolution.widthFloat(), sasSettings::singletonPtr()->_frameBufferResolution.heightFloat(), 
                                                  &matView, &matProj, true );
    


    _sasDevice->setRenderTarget( 0, normalRT );
    _sasDevice->setRenderTarget( 1, diffuseRT );
    _sasDevice->setRenderTarget( 2, velocityRT );
    _sasDevice->setDepthStencilTarget( _sasDevice->depthBuffer(), false );
    _sasDevice->setViewport( normalRT );

    _editor->frameInitialize();

    //light update
    _lightPass->updateLightData( frustum );

    //Main frame visibility resolve
    sasVisibility::sasVisibilityQueryId visibilityResolveId;
    visibilityResolveId = sasVisibility::singleton().StartVisibilityQuery( e_CullType_Frustum, 0, &frustum, _mainDrawList );
    sasVisibility::singleton().GetQueryResults( visibilityResolveId );  

    //Clear after particle gathering
    _sasDevice->clearDepthStencilTarget( _sasDevice->depthBuffer(), sasClearFlag_DepthStencil, 1.f, 0 );

    //Render draw list
    {
      INSERT_PIX_EVENT( "PIXEvent: G-Buffer creation", 0xffffffff );
      INSERT_GPU_TIMER( GBufferPass, "GBufferPass" );

      if( !sasDebugMng::singletonPtr()->newStyleRenderLists() )
        sasRenderList::renderDrawListImmediate( _mainDrawList, sasRenderList_GBufferPass, &matProjView, _constantBuffer );
      else
      {
        _mainRenderList->processList( _mainDrawList, sasRenderList_GBufferPass, &matProjView, camera->position() );

        if( _stereo->enabled() )
        {
          _stereo->updateViewData( sasStereoView::LeftEye );
          _mainRenderList->renderList( _constantBuffer );

          _stereo->updateViewData( sasStereoView::RightEye );
          _mainRenderList->renderList( _constantBuffer );

          _stereo->updateViewData( sasStereoView::Mono );
        }
        else
        {
          _mainRenderList->renderList( _constantBuffer );
        }
      }

    }

    //Debug bounding volumes
    if( _debugMng->showBoundingVolumes() )
    {
      INSERT_PIX_EVENT("PIXEvent: debug - bounding volumes", 0xffffffff);
      sasRenderList::renderDrawListImmediate( _mainDrawList, 0, &matProjView, _constantBuffer, sasRenderMode_BoundingSpheres );
    }

    //Debug wireframe
    if( _debugMng->showWireframe() )
    {
      INSERT_PIX_EVENT("PIXEvent: debug - wireframe", 0xffffffff);
      _sasDevice->clearRenderTarget(diffuseRT, &clearColor);
      sasRenderList::renderDrawListImmediate( _mainDrawList, 0, &matProjView, _constantBuffer, sasRenderMode_Wireframe );
    }

    _sasDevice->setStructuredBuffer( 1, static_cast<sasStructuredBuffer*>(NULL), sasDeviceUnit::HullShader );
    _sasDevice->setStructuredBuffer( 1, static_cast<sasStructuredBuffer*>(NULL), sasDeviceUnit::DomainShader );
    _sasDevice->flushStates();
  }  

  // unbound 
  
  _sasDevice->setRenderTarget( 0, NULL );
  _sasDevice->setRenderTarget( 1, NULL );
  _sasDevice->setRenderTarget( 2, NULL );
  _sasDevice->setRenderTarget( 3, NULL );
  _sasDevice->setDepthStencilTarget( NULL, false );

  if( _renderSettings->_shadowsEnabled )
  {
    INSERT_GPU_TIMER( ScreenSpaceShadows, "ScreenSpaceShadows" );
    _shadowMng->computeScreenSpaceShadows( normalRT, _sasDevice->depthBuffer() );
  }

  {
    sasColor clearColor;
    clearColor.set( 1.f, 1.f, 1.f );
    _sasDevice->clearRenderTarget( ssaoRT, &clearColor );
  }

  {
    INSERT_PIX_EVENT("PIXEvent: Light pass", 0xffffffff);
    INSERT_GPU_TIMER( LightPass, "LightPass" );
    _lightPass->performLightPass( lightRT, specRT, normalRT, _sasDevice->depthBuffer(), ssaoRT, _shadowMng->screenSpaceShadowMap() );
  }

  if( _renderSettings->_shadowsEnabled )
  {
    if( _renderSettings->_godraysEnabled )
    {
      INSERT_GPU_TIMER( Godrays, "Godrays" );
      _shadowMng->computeGodrays( _sasDevice->depthBuffer(), volLightRT );
    }
  }

  sasRenderTarget* pingRT = _rtPool->RequestAndLock( rtDesc );
  sasRenderTarget* pongRT = _rtPool->RequestAndLock( rtDesc );
  sasPingPongRT pingPongRTs( pingRT, pingRT );

  {
    INSERT_PIX_EVENT("PIXEvent: Composite pass", 0xffffffff);
    INSERT_GPU_TIMER( CompositePass, "CompositePass" );
    _compositePass->applyCompositePass( diffuseRT, lightRT, specRT, _shadowMng->godRayBuffer(), _sasDevice->depthBuffer(), pingPongRTs.destinationRT() );
    pingPongRTs.swapBuffers();
  }

  if( sasSplashScreen::getActiveSplashScreen() != NULL )
  {
    sasSplashScreen::getActiveSplashScreen()->render( pingPongRTs.sourceRT() );
  }
  else
  {
    sasSplashScreen::renderWidgets( pingPongRTs.sourceRT() );
  }

  if( false && _stereo->enabled() )
  {
    INSERT_PIX_EVENT("PIXEvent: OVRDIST", 0xffffffff);
    INSERT_GPU_TIMER( OVRDIST, "OVRDIST" );

    _stereo->applyDistortionCorrection( pingPongRTs.sourceRT(), pingPongRTs.destinationRT() );
    pingPongRTs.swapBuffers();
  }

  if( _renderSettings->_fxaaEnabled )
  {
    INSERT_GPU_TIMER( FXAA, "FXAA" );
    _postAA->computePostAA( pingPongRTs.sourceRT(), pingPongRTs.destinationRT() );
    pingPongRTs.swapBuffers();
  }

  //must happen before debug options are applied
  _gpuProfiler->endGpuTimer( frameTimeGpuEvenName );

  {
    INSERT_PIX_EVENT("PIXEvent: debug - Debug RT copy", 0xffffffff);
    _debugMng->applyDebugOptions( diffuseRT, normalRT, lightRT, specRT, ssaoRT, velocityRT, volLightRT, pingPongRTs.sourceRT() );
  }

  {
    INSERT_PIX_EVENT("PIXEvent: debug - Debug objects", 0xffffffff);
    _debugMng->renderDebugObjects( camera, pingPongRTs.sourceRT(), _sasDevice->depthBuffer() );
  }

  {
    INSERT_PIX_EVENT("PIXEvent: debugRender flush", 0xffffffff);
    _debugRender->flushQueuedDebugObjects( pingPongRTs.sourceRT(), _sasDevice->depthBuffer() );
  }

  _editor->runComputeJobs();
  _editor->frameEnd();

  _shadowMng->frameEnd();
  _gpuProfiler->processFrameTimings();

  sasGui::singleton().render();

  if( _stereo->enabled() )
    _stereo->vrPresentFrame( pingPongRTs.sourceRT() );
  else
  {
    copyRenderTarget( pingPongRTs.sourceRT(), _sasDevice->backBuffer(), sasSamplerState_Bilinear_Clamp );
    _sasDevice->presentFrame();
  }

  _rtPool->Unlock( pingRT );
  _rtPool->Unlock( pongRT );
  _rtPool->Unlock( volLightRT );
  _rtPool->Unlock( cubeMapRT );
  _rtPool->Unlock( normalRT );
  _rtPool->Unlock( diffuseRT );
  _rtPool->Unlock( velocityRT );
  _rtPool->Unlock( lightRT );
  _rtPool->Unlock( specRT );  
  _rtPool->Unlock( ssaoRT );
}

void sasRenderMng::copyRenderTarget(sasRenderTarget* src, sasRenderTarget* dst, sasSamplerState samplerState )
{
  INSERT_PIX_EVENT("PIXEvent: Copy render target", 0xffffffff);
  _sasDevice->setVertexShader( screenCopyShaderID );
  _sasDevice->setPixelShader( screenCopyShaderID );
  _sasDevice->setVertexBuffer( _miscResources->getFullScreenQuadVB() );
  _sasDevice->setTexture( 0, src, sasDeviceUnit::PixelShader );
  _sasDevice->setSamplerState( 0, samplerState, sasDeviceUnit::PixelShader );
  _sasDevice->setRenderTarget( 0, dst );
  _sasDevice->setViewport( dst );
  _sasDevice->setDepthStencilTarget( NULL, false );
  _sasDevice->draw( sasPrimTopology_TriStrip, 4 );
  _sasDevice->setTexture( 0, static_cast< sasTexture* >( NULL ), sasDeviceUnit::PixelShader );
  _sasDevice->flushStates();
}

void sasRenderMng::copyDepthTargetToRenderTarget( sasDepthStencilTarget* src, sasRenderTarget* dst )
{
  INSERT_PIX_EVENT("PIXEvent: Copy depth buffer to render target", 0xffffffff);
  _sasDevice->setVertexShader( screenCopyShaderID );
  _sasDevice->setPixelShader( screenCopyShaderID );
  _sasDevice->setVertexBuffer( _miscResources->getFullScreenQuadVB() );
  _sasDevice->setTexture( 0, src, sasDeviceUnit::PixelShader );
  _sasDevice->setSamplerState( 0, sasSamplerState_Point_Clamp, sasDeviceUnit::PixelShader );
  _sasDevice->setRenderTarget( 0, dst );
  _sasDevice->setViewport( dst );
  _sasDevice->setDepthStencilTarget( NULL, false );
  _sasDevice->draw( sasPrimTopology_TriStrip, 4 );
  _sasDevice->setTexture( 0, static_cast< sasTexture* >( NULL ), sasDeviceUnit::PixelShader );
  _sasDevice->flushStates();
}


void sasRenderMng::setCSLightPassState(bool enable)
{
  sasDelete _lightPass;

  if( enable )
  {
    _lightPass = sasNew sasLightPass_cs();
  }
  else
  {
    _lightPass = sasNew sasLightPass_ps();
  }
}

void sasRenderMng::renderBoundingSphere( const smMat44& xform, const smVec4& colour )
{
  sasPerInstanceConstants* cbData = static_cast< sasPerInstanceConstants* >( _constantBuffer->map() );
  if( cbData )
  {
    cbData->_worldMtx = xform;
    cbData->_lastFrameWorldMtx = xform;
    cbData->_instanceId = smVec4( 0.f );
    cbData->_tintColour = colour;
  }
  _constantBuffer->unmap();

  _sasDevice->setIndexBuffer( _miscResources->getUnitSphereIB() );        
  _sasDevice->setVertexBuffer( _miscResources->getUnitSphereVB() );      
  _sasDevice->setConstantBuffer( 1, _constantBuffer, sasDeviceUnit::VertexShader );  
  _sasDevice->setConstantBuffer( 1, _constantBuffer, sasDeviceUnit::PixelShader );   

  _sasDevice->setVertexShader( defaultShaderID|debugDiffuseOnlyMask );
  _sasDevice->setPixelShader( defaultShaderID|debugDiffuseOnlyMask );
  _sasDevice->drawIndexed( sasPrimTopology_TriList, _miscResources->getUnitSphereIB()->indicesCount() / 3 );
}

void sasRenderMng::loadEngineResources()
{
  _miscResources->loadMiscTextures();
}

