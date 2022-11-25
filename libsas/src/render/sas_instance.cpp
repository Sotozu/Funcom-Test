#include "sas_pch.h"
#include "sas_instance.h"
#include "sas_geometry.h"
#include "sas_mesh.h"
#include "sas_structuredbuffer.h"
#include "animation/sas_skeleton.h"
#include "render/debug/sas_debugrender.h"
#include "utils/sas_color.h"
#include "sas_rendertypes.h"

/*
        staging instance class
*/
sasStagingInstance::sasStagingInstance( sasGeometry* geometry )
  : _geometry( geometry )
{
  sasASSERT( _geometry );
}

sasStagingInstance::~sasStagingInstance()
{}

const smVec4* sasStagingInstance::lsBoundingSphere() 
{ 
  return _geometry->lod(0)->boundingSphere(); 
}


/*
        instance class
*/


sasInstance::Instances sasInstance::_instances; 
static const int kMaxBones = 128;
sasInstance::sasInstance( sasStagingInstance& inst, unsigned int uid )
  : _geometry( inst.geometry() )
  , _uid( uid )
  , _uidFloat( static_cast< float >( uid ) )
  , _parentModelInstance( nullptr )
  , _tintColour( 1.f )
  , _instancedData( nullptr )
  , _numInstances( 1 )
{
  sasASSERT( _geometry );

  setTransform( inst.transform() );

  Iterator it = std::find( _instances.begin(), _instances.end(), this );
  _instances.insert( it, this );

  //create skinning data if needed
  for( size_t i = 0; i < geometry()->lod( 0 )->subMeshCount(); i++ )
  {
    _skeletons.push_back( nullptr );

    sasStructuredBuffer* skinningData = NULL;
    if( geometry()->lod( 0 )->subMesh( i )->isSkinned() )
    {
      skinningData = sasNew sasStructuredBuffer( kMaxBones * sizeof( float ) * 16, sizeof( float ) * 16, true, false );

      //initialize matrix data
      smMat44* data = static_cast< smMat44* >( skinningData->map() );
      smMat44 identityMatrix;
      identityMatrix.setIdentity();
      for( size_t j = 0; j < kMaxBones; j++ )
      {
        data[ j ] = identityMatrix;
      }
      skinningData->unmap();
    }
    _skinningMatrices.push_back( skinningData );
  }

}

sasInstance::~sasInstance()
{
  for( size_t smi = 0; smi < _skinningMatrices.size(); smi++ )
  {
    if( _skinningMatrices[ smi ] )
      sasDelete _skinningMatrices[ smi ];
  }
  _skinningMatrices.clear();

  if( _instancedData )
      sasDelete _instancedData;

  //remove from instance list
  for( size_t i = 0; i < _instances.size(); i++ )
  {
    if( _instances[ i ] == this )
    {
      _instances[ i ] = _instances[ _instances.size() - 1 ];
      _instances.pop_back();
      break;
    }
  }
}

const smVec4* sasInstance::lsBoundingSphere() 
{ 
  return _geometry->lod(0)->boundingSphere(); 
}

void sasInstance::updateInstancePostFrame()
{
  updateLastFrameData();
}

void sasInstance::updateLastFrameData()
{
  _previousFrameWorldMatrix = _worldMatrix;
}


void sasInstance::setTransform( const smMat44& m ) 
{ 
  _transform = m; 
  _instanceTranform = m;
  _worldMatrix = m;
  smVec4 bsPos = *lsBoundingSphere();
  bsPos.W = 1.f;
  smMul( m, bsPos, _wsBoundingShphere ); 

  //scale application
  smVec4 diagonal( 1.f, 1.f, 1.f, 1.f );
  smNormalize3( diagonal, diagonal );
  diagonal *= lsBoundingSphere()->W;
  smVec4 extent = *lsBoundingSphere() + diagonal;
  extent.W = 1.f;
  smMul( m, extent, extent );
  float radius = smLength3( extent - _wsBoundingShphere );

  _wsBoundingShphere.W = radius;
}

void sasInstance::setInstanceTransform( const smMat44& m )   
{ 
  smMul( _transform, m, _instanceTranform );
  _worldMatrix = _instanceTranform;
  smVec4 bsPos = *lsBoundingSphere();
  bsPos.W = 1.f;
  smMul( _instanceTranform, bsPos, _wsBoundingShphere ); 
  
  //scale application
  smVec4 diagonal( 1.f, 1.f, 1.f, 1.f );
  smNormalize3( diagonal, diagonal );
  diagonal *= lsBoundingSphere()->W;
  smVec4 extent = *lsBoundingSphere() + diagonal;
  extent.W = 1.f;
  smMul( _instanceTranform, extent, extent );
  float radius = smLength3( extent - _wsBoundingShphere );

  _wsBoundingShphere.W = radius;
}

//deprecated method, do not use
void sasInstance::setFrameTransform( const smMat44& m )
{
  sasASSERT( false ); //make sure this is no longer used
  _previousFrameWorldMatrix = _worldMatrix;
  smMul( m, _instanceTranform, _worldMatrix );
  smVec4 bsPos = *lsBoundingSphere();
  bsPos.W = 1.f;
  smMul( _worldMatrix, bsPos, _wsBoundingShphere ); 
  _wsBoundingShphere.W = lsBoundingSphere()->W;
}

void sasInstance::updateSkinningMatrices( size_t subMeshIndex, const sasSkeleton* skeleton )
{
  sasASSERT( subMeshIndex < _geometry->lod( 0 )->subMeshCount() );
  sasStructuredBuffer* skinningData = _skinningMatrices[ subMeshIndex ];
  
  //cached ptr for debugging purposes, not used for animation
  _skeletons[ subMeshIndex ] = skeleton;

  if( skinningData )
  {
    smMat44* data = static_cast< smMat44* >( skinningData->map() );
    for( size_t i = 0; i < skeleton->_numBones; i++ )
    {
      data[ i ] = skeleton->_transformedMatrices[ i ];
    }
    skinningData->unmap();
  }
}

void sasInstance::renderSkeleton() const
{
  for( size_t s = 0; s < _skeletons.size(); s++ )
  {
    if( _skeletons[ s ] )
    {
      const sasSkeleton* sk = _skeletons[ s ];
      for( uint32_t i = 0; i < sk->_numBones; i++ )
      {
        //current bone
        smVec4 bonePos = sk->getBoneWorldPosition( sk->_boneIdArray[ i ] );
        smMul( _worldMatrix, bonePos, bonePos );
        sasDebugRender::singletonPtr()->renderSphere( bonePos, 0.05f, smVec4( 0.2f, 0.f, 1.f, 0.6f ) );

        //parent bone
        sasStringId parentBoneId = sk->getParentBoneId( sk->_boneIdArray[ i ] );
        uint32_t index;
        bool exists = sk->findBoneIndexSafe( parentBoneId, index );
        if( exists )
        {
          //if there is a parent, draw the bone connecting line
          smVec4 parentBonePos = sk->getBoneWorldPosition( parentBoneId );
          smMul( _worldMatrix, parentBonePos, parentBonePos );
          sasDebugRender::singletonPtr()->renderLine( bonePos, parentBonePos, sasColor( 0.1f, 0.1f, 1.f, 1.f ), sasDepthStencilState_NoZTest );
        }

      }
    }
  }

}

const sasSkeleton* sasInstance::skeleton( unsigned int submeshIndex ) const
{
  sasASSERT( submeshIndex < _skeletons.size() );
  return _skeletons[ submeshIndex ];
}

uint32_t sasInstance::numSubmeshes() const
{
  return _geometry->lod( 0 )->subMeshCount();
}

bool sasInstance::getBoneWorldPosition( sasStringId boneId, smVec3& pos ) const
{
  for( size_t s = 0; s < _skeletons.size(); s++ )
  {
    if( _skeletons[ s ] )
    {
      const sasSkeleton* sk = _skeletons[ s ];
      if( sk->getBoneWorldPositionSafe( boneId, pos ) )
        return true;
    }
  }
  return false;
}

void sasInstance::setInstancedData( void* data, uint32_t size )
{
    if( _instancedData == nullptr )
    {
        _instancedData = sasNew sasStructuredBuffer( size, sizeof( float ) * 4, true, false );
    }

    void* destData = static_cast< void* >( _instancedData->map() );
    memcpy( destData, data, size );
    _instancedData->unmap();

    _wsBoundingShphere = smVec4( 0.f, 0.f, 0.f, 1000.f ); //hack - inflate bounding sphere to always be visible
}
