#include "sas_pch.h"
#include "sas_modelinstance.h"
#include "sas_resourcemng.h"
#include "sas_resource.h"
#include "sas_model_resource.h"
#include "sas_resource.h"
#include "render/sas_instance.h"
#include "animation/sas_skeleton.h"
#include "utils/sas_timer.h"


sasModelInstance::sasModelInstance( sasModelResource* modelResource, sasStringId instanceId )
    : _modelResource( modelResource )
    , _instanceId( instanceId )
    , _dirty( true )
    , _position( 0.f )
    , _rotation( 0.f )
    , _scale( 1.f )
    , _skinned( false )
    , _animationPaused( false )
    , _tintColour( 1.f )
{
  static unsigned int s_instanceUid = 0;
  const sasModelResourceData res_data = modelResource->resourceData();
  for( size_t i = 0; i < res_data.instances.size(); i++ )
  {
    sasInstance* inst = sasNew sasInstance( *( res_data.instances[ i ] ), ++s_instanceUid );
    inst->setParentModelInstance( this );
    _instances.push_back( inst );

    
    
  }

  if( res_data.skeletonHierarchy != nullptr )
    _skinned = true;

  _animLayers.reserve( 4 );
}

sasModelInstance::~sasModelInstance()
{
  for( size_t i = 0; i < _instances.size(); i++ )
  {
   

    sasDelete _instances[ i ];
  }
  _instances.clear();

  _modelResource->releaseRefCount();
}

void sasModelInstance::renderSkeleton() const
{
  if( _skinned )
  {
    for( size_t i = 0; i < _instances.size(); i++ )
    {
      _instances[ i ]->renderSkeleton();
    }
  }
}

const smMat44& sasModelInstance::transform()
{
  if( _dirty )
  {
    _dirty = false;
    smEulerAngles rot;
    rot.Pitch = _rotation.X;
    rot.Yaw = _rotation.Y;
    rot.Roll = _rotation.Z;
    smGetXFormTRSMatrix( _position.X, _position.Y, _position.Z, rot, _scale.X, _scale.Y, _scale.Z, _transform );
  }
  return _transform;
}

void sasModelInstance::addAnimationLayer( const sasStringId& animId, const sasStringId& boneSetMask, float animTime, float blendCoeff )
{
  sasAnimationLayer al;
  al._animId = animId;
  al._animTime = animTime;
  al._blendAmnt = blendCoeff;
  al._boneSetMask = boneSetMask;
  _animLayers.push_back( al );
}

void sasModelInstance::updateInstances()
{
  for( size_t i = 0; i < _instances.size(); i++ )
  {
    _instances[ i ]->setInstanceTransform( transform() );
    _instances[ i ]->setTintColour( _tintColour );
  }
}

void sasModelInstance::setInstanceData( void* data, uint32_t size )
{
    for( size_t i = 0; i < _instances.size(); i++ )
    {
        _instances[ i ]->setInstancedData( data, size );
    }
}

void sasModelInstance::setInstanceCount( uint32_t numInstances )
{
    for( size_t i = 0; i < _instances.size(); i++ )
    {
        _instances[ i ]->setInstanceCount( numInstances );
    }
}

void sasModelInstance::stepPreRender()
{
  if( _skinned && !_animationPaused )
  {
    if( _animLayers.size() > 0 )
    {
      const sasAnimation* anim = sasResourceMng::singleton().getAnimation( _animLayers[ 0 ]._animId );

      if( anim )
      {
        _modelResource->resourceData().skeletonHierarchy->applyAnimation( *anim, _animLayers[ 0 ]._animTime );

        for( unsigned int i = 0; i < _modelResource->resourceData().skeletons.size(); i++ )
        {
          if( _modelResource->resourceData().skeletons[ i ] != NULL )
          {
            _modelResource->resourceData().skeletons[ i ]->applyAnimationFromSkeletonHierarchy();

            _instances[ 0 ]->updateSkinningMatrices( i, _modelResource->resourceData().skeletons[ i ] );
          }
        }
      }

      _animLayers.clear();
    }
  }
}