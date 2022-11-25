#include "sas_pch.h"
#include "sas_main.h"
#include "sas_core.h"

#include "sas_device.h"
#include "resource/sas_resourcemng.h"
#include "resource/sas_modelinstance.h"
#include "resource/sas_splashscreen.h"
#include "render/gui/sas_gui.h"
#include "render/debug/sas_debugmng.h"
#include "utils/sas_camera.h"
#include "utils/sas_stringid.h"
#include "utils/sas_timer.h"
#include "editor/sas_editor.h"
#include "utils/sas_cameramng.h"
#include "resource/sas_camerapath.h"
#include "render/sas_rendermng.h"
#include "render/debug/sas_debugrender.h"
#include "render/lights/sas_lightmng.h"
#include "render/lights/sas_light.h"

#include "render/sas_stereo.h"
#include "OVR.h"

static sasCore* s_core = 0;
static char     s_coreBuffer[ sizeof(sasCore) ];


// ----------------------------------------------------------------------------
//  Main
// ----------------------------------------------------------------------------

bool sasInitialize( const sasConfig& config )
{
  if( !s_core )
  {
    s_core = new( s_coreBuffer ) sasCore( config );
  }
  return s_core!=0;
}

void sasUpdate()
{
  sasASSERT( s_core );
  s_core->update();
}

void sasShutdown()
{
  sasASSERT( s_core );
   s_core->~sasCore();
   s_core = 0;
}


// ----------------------------------------------------------------------------
//  Resource
// ----------------------------------------------------------------------------

sasResource* sasLoadResource( const char* path )
{
  sasASSERT( s_core );
  return sasResourceMng::singleton().load( path );  
}

void sasCreateModelInstance( const char* path, sasStringId& instanceId, const char* name )
{
  sasASSERT( s_core );
  instanceId = sasResourceMng::singleton().createModelInstance( path, name );
}

void sasCreateBlobModelInstance( const sasStringId& instanceId )
{
  sasASSERT( s_core );
  sasResourceMng::singleton().createBlobModelInstance( instanceId );
}

void sasDeleteModelInstance( sasStringId instanceId )
{
  sasASSERT( s_core );
  sasResourceMng::singleton().deleteModelInstance( instanceId );
}

void sasCreateSplashScreen( sasStringId& id, const char* texturePath, const smVec2& position, const smVec2& scale )
{
  sasASSERT( s_core );
  sasResourceMng::singleton().createSplashScreen( id, texturePath, position, scale );
}

void sasUnloadResource( sasResource* resource )
{
    sasASSERT( s_core );
    return sasResourceMng::singleton().unload( resource );  
}

void sasUnloadAllResources()
{
  sasASSERT( s_core );
  sasResourceMng::singleton().unloadAll();
}

void sasReloadShaders()
{
  sasDevice::singleton().reloadShaders(); 
}

void sasSetResourceTransform( const sasStringId& hash, const smMat44& transform )
{
  sasASSERT( false ); //unimplemented
}


// ----------------------------------------------------------------------------
//  Model Resource
// ----------------------------------------------------------------------------

void sasSetModelInstanceData( const sasStringId& hash, void* data, uint32_t size, uint32_t instanceCount )
{
  sasASSERT( s_core );
  sasModelInstance* inst = sasResourceMng::singleton().findModelInstance( hash );
  if( inst )
  {
    inst->setInstanceData( data, size );
    inst->setInstanceCount( instanceCount );
  }
}

void sasSetInstancePosition( const sasStringId& hash, const smVec3& position )
{
  sasASSERT( s_core );
  sasModelInstance* inst = sasResourceMng::singleton().findModelInstance( hash );
  if( inst )
    inst->setPosition( position );
}

void sasSetInstanceRotation( const sasStringId& hash, const smVec3& rotation )
{
  sasASSERT( s_core );
  sasModelInstance* inst = sasResourceMng::singleton().findModelInstance( hash );
  if( inst )
    inst->setRotation( rotation );
}

void sasSetInstanceScale( const sasStringId& hash, const smVec3& scale )
{
  sasASSERT( s_core );
  sasModelInstance* inst = sasResourceMng::singleton().findModelInstance( hash );
  if( inst )
    inst->setScale( scale );
}

void sasSetInstanceTintColour( const sasStringId& hash, const smVec4& tintColour )
{
  sasASSERT( s_core );
  sasModelInstance* inst = sasResourceMng::singleton().findModelInstance( hash );
  if( inst )
    inst->setTintColour( tintColour );
}

void sasSetInstanceVisible( const sasStringId& hash, bool visible )
{
  sasASSERT( s_core );
  sasModelInstance* inst = sasResourceMng::singleton().findModelInstance( hash );
  if( inst )
    inst->setVisible( visible );
}

void sasAddAnimationLayer( const sasStringId& hash, const sasStringId& animId, const sasStringId& boneSetMask, float animTime, float blendCoeff )
{
  sasASSERT( s_core );
  sasModelInstance* inst = sasResourceMng::singleton().findModelInstance( hash );
  if( inst )
  {
    inst->addAnimationLayer( animId, boneSetMask, animTime, blendCoeff );
  }
}

void sasSetAnimationPaused( const sasStringId& hash, bool state )
{
  sasASSERT( s_core );
  sasModelInstance* inst = sasResourceMng::singleton().findModelInstance( hash );
  if( inst )
  {
    inst->setAnimationPaused( state );
  }
}


// ----------------------------------------------------------------------------
//  UI
// ----------------------------------------------------------------------------

void sasSetSplashScreen( sasStringId& id )
{
  sasASSERT( s_core );
  sasSplashScreen::setActiveSplashScreen( sasResourceMng::singleton().findSplashScreen( id ) );
}

void sasUnbindSplashScreen()
{
  sasASSERT( s_core );
  sasSplashScreen::setActiveSplashScreen( NULL );
}

void sasActivateWidget( sasStringId widget )
{
  sasASSERT( s_core );
  sasSplashScreen* w = sasResourceMng::singleton().findSplashScreen( widget );
  if( w )
    sasSplashScreen::activateWidget( w );
}

void sasDeactivateWidget( sasStringId widget )
{
  sasASSERT( s_core );
  sasSplashScreen* w = sasResourceMng::singleton().findSplashScreen( widget );
  if( w )
    sasSplashScreen::deactivateWidget( w );
}


// ----------------------------------------------------------------------------
//  Camera
// ----------------------------------------------------------------------------

sasCamera* sasCreateCamera()
{
  sasASSERT( s_core );
  sasCamera* camera = sasNew sasCamera();
  sasCameraMng::singletonPtr()->addCamera( camera );
  return camera;
}

void sasDestroyCamera( sasCamera* camera )
{
  sasCameraMng::singletonPtr()->removeCamera( camera );
  sasASSERT( s_core );
  sasDelete camera;
}

smVec4 sasCameraPosition( const sasCamera* camera )
{
  sasASSERT( s_core && camera );
  return camera->position();
}

smVec4 sasCameraFront( const sasCamera* camera )
{
  sasASSERT( s_core && camera );
  return camera->front();
}

smVec4 sasCameraRight( const sasCamera* camera )
{
  sasASSERT( s_core && camera );
  smVec4 r;
  camera->getRight( r );
  return r;
}

smVec4 sasCameraUp( const sasCamera* camera )
{
    sasASSERT( s_core && camera );
    smVec4 r = camera->up();
    return r;
}

void sasCameraSetPosition( const smVec4& p, sasCamera* camera )
{
  sasASSERT( s_core && camera );
  camera->setPosition( p );
}

void sasCameraLookAt( const smVec4& target, const smVec4& up, sasCamera* camera )
{
  sasASSERT( s_core && camera );
  camera->lookAt( target, up );
}


// ----------------------------------------------------------------------------
//  Input
// ----------------------------------------------------------------------------

bool sasApplyInput( const sasInput& input )
{
  if( sasGui::isValid() )
    return sasGui::singleton().applyInput(input);

  return false;
}

// void sasSetOVRData( const OVR::Util::Render::StereoConfig& ovrConfig )
// {
//   sasStereo::singletonPtr()->setOvrSteroConfig( ovrConfig );
// }

void sasToggleOVRMode( const std::string& displayName, unsigned int displayId )
{
  static float lastTimeToggled = 0.f;
  float time = sasTimer::singletonPtr()->getTime();
  if( time - lastTimeToggled < 1.f )
    return;

  lastTimeToggled = time;

  static bool enabled = false;
  enabled = !enabled;

  if( enabled )
  {
    if( displayId != 0xffffffff )
    {
      //VR mode, so disable debug gui, enter fullscreen, target VR display, flush frame every present
      sasDebugOutput( "Entering VR mode...\n" );

      sasDevice::singletonPtr()->setFullscreenMode( 1920, 1080, displayName, enabled );
      sasRenderMng::singletonPtr()->setUIEnabled( false );
      //sasDebugMng::singletonPtr()->setVSyncEnabled( true );
      //sasDebugMng::singletonPtr()->setForceGPUFlush( true );
      //sasDebugMng::singletonPtr()->setStereoEnabled( true );
      //sasDebugMng::singletonPtr()->setDistortionCompPassEnabled( true );
      sasDebugMng::singletonPtr()->setMotionBlurEnabled( false );
      sasDebugMng::singletonPtr()->setVolumetricLightingEnabled( false );
    }
  }
  else
  {
    sasDebugOutput( "Exiting VR mode...\n" );
    //no VR, so enable gui, exit full screen
    sasDevice::singletonPtr()->setFullscreenMode( 1280, 800, displayName, false );
    sasRenderMng::singletonPtr()->setUIEnabled( true );
    sasDebugMng::singletonPtr()->setVSyncEnabled( false );
    sasDebugMng::singletonPtr()->setForceGPUFlush( false );
    sasDebugMng::singletonPtr()->setStereoEnabled( false );
    sasDebugMng::singletonPtr()->setDistortionCompPassEnabled( false );
  }
}

// ----------------------------------------------------------------------------
//  Misc
// ----------------------------------------------------------------------------

void sasCastRay(unsigned int x, unsigned int y )
{
}

float sasGetTime()
{
  return sasTimer::singletonPtr()->getTime();
}

bool sasGetControllerInverted()
{
  return sasDebugMng::singletonPtr()->controllerInverted();
}

bool sasGetMouseInverted()
{
  return sasDebugMng::singletonPtr()->mouseInverted();
}

void sasSetCinematicMode( bool state )
{
  sasRenderMng::singletonPtr()->setUIEnabled( !state );
}

uint32_t sasGetFrameBufferWidth()
{
  return sasDevice::singletonPtr()->backBufferWidth();
}

uint32_t sasGetFrameBufferHeight()
{
  return sasDevice::singletonPtr()->backBufferHeight();
}


// ----------------------------------------------------------------------------
//  Editor
// ----------------------------------------------------------------------------

void sasToggleEditorMode()
{
  static float lastTimeToggled = 0.f;
  float time = sasTimer::singletonPtr()->getTime();
  if( time - lastTimeToggled < 1.f )
    return;

  lastTimeToggled = time;

  if( sasDebugMng::singletonPtr()->editMode() )
    sasDebugMng::singletonPtr()->setEditMode( false );
  else
    sasDebugMng::singletonPtr()->setEditMode( true );
}

void sasPickMeshInstance( unsigned int x, unsigned int y )
{
  sasEditor::singletonPtr()->kickMeshPickingTask( x, y );
}

void sasPickBoneForInstance( unsigned int x, unsigned int y, sasStringId modelId, uint32_t meshId )
{
  sasEditor::singletonPtr()->kickBonePickTaskForInstance( x, y, modelId, meshId );
}

bool sasGetPickedMeshData( sasPickedMeshData& meshData )
{
  meshData = sasEditor::singletonPtr()->pickedMeshData();
  return sasEditor::singletonPtr()->areMeshPickingResultsReady();
}

bool sasGetPickedBoneData( sasPickedBoneData& boneData )
{
  boneData = sasEditor::singletonPtr()->pickedBoneData();
  return sasEditor::singletonPtr()->areBonePickingResultsReady();
}

// ----------------------------------------------------------------------------
//  Editor
// ----------------------------------------------------------------------------

void sasRenderDebugSphere( const smVec3& pos, float radius, const smVec4& colour, bool depthTested, bool wireFrame )
{
  sasDebugSphere s;
  s.depthState = depthTested ? sasDepthStencilState_LessEqual : sasDepthStencilState_NoZTest;
  s.rasterState = wireFrame ? sasRasterizerState_Wireframe_NoCull : sasRasterizerState_Solid;
  s._col = colour;
  s._pos = pos;
  s._radius = radius;
  sasDebugRender::singletonPtr()->queueRenderSphere( s );
}

void sasRenderDebugLine( const smVec3& posA, const smVec3& posB, const smVec4& colour, bool depthTested )
{
  sasDebugLine l;
  l.depthState = depthTested ? sasDepthStencilState_LessEqual : sasDepthStencilState_NoZTest;
  l._col = colour;
  l._posA = posA;
  l._posB = posB;
  sasDebugRender::singletonPtr()->queueRenderLine( l );
}

void sasRenderDebugSkeleton( const sasStringId& modelInstanceId )
{
  sasDebugRender::singletonPtr()->queueRenderSkeleton( modelInstanceId );
}

void sasRenderDebugBone( const sasStringId& modelInstanceId, const sasStringId& boneId )
{
  sasDebugRender::singletonPtr()->queueRenderBone( modelInstanceId, boneId );
}

void sasRenderDebugBox( const smAABB& aabb, const smMat44& xform, const smVec4& colour, bool depthTested, bool wireFrame )
{
  sasDebugBox b;
  b.depthState = depthTested ? sasDepthStencilState_LessEqual : sasDepthStencilState_NoZTest;
  b.rasterState = wireFrame ? sasRasterizerState_Wireframe_NoCull : sasRasterizerState_Solid;
  b._col = colour;
  b._xform = xform;
  b._aabb = aabb;
  sasDebugRender::singletonPtr()->queueRenderBox( b );
}



// ----------------------------------------------------------------------------
//  Lights
// ----------------------------------------------------------------------------

void sasCreatePointLight( sasStringId lightId, const smVec3& pos, float radius, const smVec3& colour, bool enabled )
{
  sasLightMng::singletonPtr()->createPointLight( lightId, pos, radius, colour, enabled );
}

void sasCreateSpotLight( sasStringId lightId, const smVec3& position, const smVec3& direction, float innerAngle, float outerAngle, float radius, const smVec4& colour, bool enabled )
{
  sasLightMng::singletonPtr()->createSpotLight( lightId, position, direction, innerAngle, outerAngle, radius, colour, enabled );
}

void sasDestroyPointLight( sasStringId lightId )
{
  sasLightMng::singletonPtr()->deletePointLight( lightId );
}

void sasDestroySpotLight( sasStringId lightId )
{
  sasLightMng::singletonPtr()->deleteSpotLight( lightId );
}

void sasSetLightPosition( sasLightType::Enum type, sasStringId lightId, const smVec3& position )
{
  if( type == sasLightType::Point )
  {
    sasPointLight* l = sasLightMng::singletonPtr()->getPointLight( lightId );
    if( l )
      l->setPosition( position );
  }
  else if( type == sasLightType::Spot )
  {
    sasSpotLight* l = sasLightMng::singletonPtr()->getSpotLight( lightId );
    if( l )
      l->setPosition( position );
  }
}

void sasSetLightColour( sasLightType::Enum type, sasStringId lightId, const smVec3& colour )
{
  if( type == sasLightType::Point )
  {
    sasPointLight* l = sasLightMng::singletonPtr()->getPointLight( lightId );
    if( l )
      l->setColour( colour );
  }
  else if( type == sasLightType::Spot )
  {
    sasSpotLight* l = sasLightMng::singletonPtr()->getSpotLight( lightId );
    if( l )
      l->setColour( colour );
  }
}

void sasSetLightRange( sasLightType::Enum type, sasStringId lightId, float range )
{
  if( type == sasLightType::Point )
  {
    sasPointLight* l = sasLightMng::singletonPtr()->getPointLight( lightId );
    if( l )
      l->setRadius( range );
  }
  else if( type == sasLightType::Spot )
  {
    sasSpotLight* l = sasLightMng::singletonPtr()->getSpotLight( lightId );
    if( l )
      l->setRadius( range );
  }
}

void sasSetSpotLightProperties( sasStringId lightId, const smVec3& direction, float innerAngle, float outerAngle )
{
  sasSpotLight* l = sasLightMng::singletonPtr()->getSpotLight( lightId );
  if( l )
  {
    l->setDirection( direction );
    l->setInnerAngle( innerAngle );
    l->setOuterAngle( outerAngle );
  }
}

void sasSetLightEnabled( sasLightType::Enum type, sasStringId lightId, bool enabled )
{
  if( type == sasLightType::Point )
  {
    sasPointLight* l = sasLightMng::singletonPtr()->getPointLight( lightId );
    if( l )
      l->setEnabled( enabled );
  }
  else if( type == sasLightType::Spot )
  {
    sasSpotLight* l = sasLightMng::singletonPtr()->getSpotLight( lightId );
    if( l )
      l->setEnabled( enabled );
  }
}

void sasSetLightShadowCaster( sasLightType::Enum type, sasStringId lightId, bool enabled )
{
  if( type == sasLightType::Spot )
  {
    sasSpotLight* l = sasLightMng::singletonPtr()->getSpotLight( lightId );
    if( l )
      l->setCastsShadows( enabled );
  }
}


// ----------------------------------------------------------------------------
//  Particles
// ----------------------------------------------------------------------------

void sasCreateParticleEffect( sasStringId emitterId, const char* name, const sasParticleEmitterData& emitterData )
{
  
}

void sasDestroyParticleEffect( sasStringId emitterId )
{

}

void sasRunParticleEffectBurst( sasStringId emitterId, bool endless )
{

}

void sasSetParticleEffectPosition( sasStringId emitterId, const smVec3& pos )
{

}

void sasSetParticleEffectDirection( sasStringId emitterId, const smVec3& dir )
{

}

void sasUpdateParticleEffectProperties( sasStringId emitterId, const sasParticleEmitterData& emitterData )
{
  
}

