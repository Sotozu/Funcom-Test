#pragma once 

#include "sas_export.h"
#include <sasMaths.h>
#include <string>
#include "su_main.h"
#include "utils/sas_stringid.h"

class sasResource;
class sasCamera;

namespace OVR
{
  namespace Util
  {
    namespace Render
    {
      class StereoConfig;
    }
  }
}

// ----------------------------------------------------------------------------
//  Types
// ----------------------------------------------------------------------------

struct sasInput
{
  void*         hwnd;
	unsigned int  msg;
	unsigned int  wParam;
	unsigned long lParam;

  sasInput()
    : hwnd(0)
    , msg(0)
    , wParam(0)
    , lParam(0)
  {}
};

struct sasConfig
{
  void*       hwnd;
  const char* shaderFolder;
  const char* resourcePath;
  const void* vrHmd;

  sasConfig()
	: hwnd( 0 )
	, shaderFolder( 0 )
	, resourcePath( 0 )
  , vrHmd( nullptr )
  {}
};

// ----------------------------------------------------------------------------
//  Main
// ----------------------------------------------------------------------------

sasAPI bool sasInitialize( const sasConfig& config );
sasAPI void sasUpdate();
sasAPI void sasShutdown();


// ----------------------------------------------------------------------------
//  Resource
// ----------------------------------------------------------------------------

sasAPI sasResource* sasLoadResource( const char* path ); 
sasAPI void sasCreateModelInstance( const char* path, sasStringId& instanceId, const char* name );
sasAPI void sasCreateBlobModelInstance( const sasStringId& instanceId );
sasAPI void sasDeleteModelInstance( sasStringId instanceId );
sasAPI void sasCreateSplashScreen( sasStringId& id, const char* texturePath, const smVec2& position, const smVec2& scale );
sasAPI void sasUnloadResource( sasResource* resource );
sasAPI void sasUnloadAllResources();
sasAPI void sasReloadShaders();
sasAPI void sasCreatePointLight( sasStringId lightId, const smVec3& pos, float radius, const smVec3& colour, bool enabled );
sasAPI void sasCreateSpotLight( sasStringId lightId, const smVec3& position, const smVec3& direction, float innerAngle, float outerAngle, float radius, const smVec4& colour, bool enabled );
sasAPI void sasDestroyPointLight( sasStringId lightId );
sasAPI void sasDestroySpotLight( sasStringId lightId );


// ----------------------------------------------------------------------------
//  Lights
// ----------------------------------------------------------------------------

namespace sasLightType
{
  enum Enum
  {
    Point = 0,
    Spot,
  };
}

sasAPI void sasSetLightPosition( sasLightType::Enum type, sasStringId lightId, const smVec3& position );
sasAPI void sasSetLightColour( sasLightType::Enum type, sasStringId lightId, const smVec3& colour );
sasAPI void sasSetLightRange( sasLightType::Enum type, sasStringId lightId, float range );
sasAPI void sasSetSpotLightProperties( sasStringId lightId, const smVec3& direction, float innerAngle, float outerAngle );
sasAPI void sasSetLightEnabled( sasLightType::Enum type, sasStringId lightId, bool enabled );
sasAPI void sasSetLightShadowCaster( sasLightType::Enum type, sasStringId lightId, bool enabled );


// ----------------------------------------------------------------------------
//  Model Resource
// ----------------------------------------------------------------------------

sasAPI void sasSetModelInstanceData( const sasStringId& hash, void* data, uint32_t size, uint32_t instanceCount );
sasAPI void sasSetInstancePosition( const sasStringId& hash, const smVec3& position );
sasAPI void sasSetInstanceRotation( const sasStringId& hash, const smVec3& rotation );
sasAPI void sasSetInstanceScale( const sasStringId& hash, const smVec3& scale );
sasAPI void sasSetInstanceVisible( const sasStringId& hash, bool visible );
sasAPI void sasSetInstanceTintColour( const sasStringId& hash, const smVec4& tintColour );
sasAPI void sasAddAnimationLayer( const sasStringId& hash, const sasStringId& animId, const sasStringId& boneSetMask, float animTime, float blendCoeff );
sasAPI void sasSetAnimationPaused( const sasStringId& hash, bool state );


// ----------------------------------------------------------------------------
//  Particles
// ----------------------------------------------------------------------------

namespace sasParticleEmitterShape
{
  enum Enum
  {
    Point = 0,
    AABB,
    Sphere
  };
}

struct sasParticleEmitterData
{
  smVec3  _position;
  smVec3  _direction;
  smVec3  _colour;
  smVec3  _boxExtents;
  float   _velocity;
  float   _particleLifetime;
  float   _velocityFudgeFactor;
  float   _burstDuration;
  float   _emissionRate;
  float   _size;
  float   _sizeScale;
  uint32_t _texture;
  int     _depthCollision : 1;
  int     _opaque : 1;
  sasParticleEmitterShape::Enum _emitterShape;
};

sasAPI void sasCreateParticleEffect( sasStringId emitterId, const char* name, const sasParticleEmitterData& emitterData );
sasAPI void sasDestroyParticleEffect( sasStringId emitterId );
sasAPI void sasRunParticleEffectBurst( sasStringId emitterId, bool endless );
sasAPI void sasSetParticleEffectPosition( sasStringId emitterId, const smVec3& pos );
sasAPI void sasSetParticleEffectDirection( sasStringId emitterId, const smVec3& dir );
sasAPI void sasUpdateParticleEffectProperties( sasStringId emitterId, const sasParticleEmitterData& emitterData );


// ----------------------------------------------------------------------------
//  UI
// ----------------------------------------------------------------------------

sasAPI void sasSetSplashScreen( sasStringId& id );
sasAPI void sasUnbindSplashScreen();
sasAPI void sasActivateWidget( sasStringId widget );
sasAPI void sasDeactivateWidget( sasStringId widget );


// ----------------------------------------------------------------------------
//  Camera
// ----------------------------------------------------------------------------

sasAPI sasCamera* sasCreateCamera();
sasAPI void sasDestroyCamera( sasCamera*  );
sasAPI smVec4 sasCameraPosition( const sasCamera* );
sasAPI smVec4 sasCameraFront( const sasCamera* );
sasAPI smVec4 sasCameraRight( const sasCamera* );
sasAPI smVec4 sasCameraUp( const sasCamera* );
sasAPI void sasCameraSetPosition( const smVec4&, sasCamera* );
sasAPI void sasCameraLookAt( const smVec4& target, const smVec4& up, sasCamera* );


// ----------------------------------------------------------------------------
//  Input
// ----------------------------------------------------------------------------

sasAPI bool sasApplyInput( const sasInput& pInput );
//sasAPI void sasSetOVRData( const OVR::Util::Render::StereoConfig& ovrConfig );
sasAPI void sasToggleOVRMode( const std::string& displayName, unsigned int displayId );


// ----------------------------------------------------------------------------
//  Misc
// ----------------------------------------------------------------------------

sasAPI void sasCastRay( unsigned int x, unsigned int y );
sasAPI float sasGetTime();
sasAPI bool sasGetControllerInverted();
sasAPI bool sasGetMouseInverted();
sasAPI void sasSetCinematicMode( bool state );
sasAPI uint32_t sasGetFrameBufferWidth();
sasAPI uint32_t sasGetFrameBufferHeight();


// ----------------------------------------------------------------------------
//  Editor
// ----------------------------------------------------------------------------

sasAPI void sasToggleEditorMode();
sasAPI void sasPickMeshInstance( unsigned int x, unsigned int y );
sasAPI void sasPickBoneForInstance( unsigned int x, unsigned int y, sasStringId modelId, uint32_t meshId );

struct sasPickedMeshData
{
  sasStringId   _modelInstanceId;
  smVec3        _modelPos;
  smVec3        _modelScale;
  smVec3        _modelRot;
  smVec4        _meshBoundingSphere;
  uint32_t      _meshId;
  uint32_t      _skinned : 1;
};
sasAPI bool sasGetPickedMeshData( sasPickedMeshData& meshData );

struct sasPickedBoneData
{
  sasStringId   _boneId;
  uint32_t      _submeshIndex;
  sasStringId   _modelInstanceId;
  uint32_t      _meshId;
};
sasAPI bool sasGetPickedBoneData( sasPickedBoneData& boneData );

// ----------------------------------------------------------------------------
//  Debug rendering
// ----------------------------------------------------------------------------

sasAPI void sasRenderDebugSphere( const smVec3& pos, float radius, const smVec4& colour, bool depthTested, bool wireFrame );
sasAPI void sasRenderDebugLine( const smVec3& posA, const smVec3& posB, const smVec4& colour, bool depthTested );
sasAPI void sasRenderDebugSkeleton( const sasStringId& modelInstanceId );
sasAPI void sasRenderDebugBone( const sasStringId& modelInstanceId, const sasStringId& boneId );
sasAPI void sasRenderDebugBox( const smAABB& aabb, const smMat44& xform, const smVec4& colour, bool depthTested, bool wireFrame );
