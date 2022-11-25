#include "sas_pch.h"
#include "sas_skeleton.h"
#include "sas_animation.h"
#include "utils/sas_virtual_file.h"

void sasSkeletonMapping::findBoneMappingForVertex( unsigned int vertexIndex, int& boneIndex0, float& boneWeight0,
                                                                              int& boneIndex1, float& boneWeight1,
                                                                              int& boneIndex2, float& boneWeight2,
                                                                              int& boneIndex3, float& boneWeight3 ) const
{
  int indices[ 4 ] = { 0, 0, 0, 0 };
  float weights[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
  unsigned int numBones = 0;
  bool maxInfluenceCountReached = false;
  for( size_t i = 0; i < _boneMappings.size(); i++ )
  {
    for( size_t j = 0; j < _boneMappings[ i ]._vertexInfluences.size(); j++ )
    {
      if( _boneMappings[ i ]._vertexInfluences[ j ]._vertexIndex == vertexIndex )
      {
        indices[ numBones ] = i;
        weights[ numBones ] = _boneMappings[ i ]._vertexInfluences[ j ]._boneWeight;
        numBones++;

        if( numBones >= 4 )
        {
          maxInfluenceCountReached = true;
          break;
        }
      }
    }
    if( maxInfluenceCountReached )
      break;
  }

  boneIndex0 = indices[ 0 ];
  boneIndex1 = indices[ 1 ];
  boneIndex2 = indices[ 2 ];
  boneIndex3 = indices[ 3 ];
  boneWeight0 = weights[ 0 ];
  boneWeight1 = weights[ 1 ];
  boneWeight2 = weights[ 2 ];
  boneWeight3 = weights[ 3 ];

}

sasSkeleton::sasSkeleton( unsigned int numBones )
  : _numBones( numBones )
{
  static const sasStringId invalidId = sasStringId::build( "invalid" );
  for( unsigned int i = 0; i < sMaxBones; i++ )
  {
    _parentBoneIdArray[ i ] = invalidId;
  }
}

sasSkeletonNode::sasSkeletonNode()
  : _numChildren( 0 )
  , _parent( NULL )
  , _boneIndex( 0 )
{
  _name = sasStringId::build( "unnamed" );
  _transform.setIdentity();
  _animatedTransform.setIdentity();
  for( int i = 0; i < kMaxNumChildrenNodes; i++ )
  {
    _children[ i ] = NULL;
  }
}

sasSkeletonHierarchy::sasSkeletonHierarchy()
  : _numNodes( 0 )
{
  _skeletonNodes = sasNew sasSkeletonNode[ 256 ];
}

sasSkeletonHierarchy::~sasSkeletonHierarchy()
{
  sasDelete [] _skeletonNodes;
}

void sasSkeletonHierarchy::applyAnimation( const sasAnimation& animation, float animationTime )
{
  smMat44 identityMatrix;
  identityMatrix.setIdentity();

  float ticksPerSecond = ( float )( animation._ticksPerSecond != 0.f ? animation._ticksPerSecond : 25.f );
  float timeInTicks = animationTime * ticksPerSecond;
  float animationTimeLooped = fmod( timeInTicks, ( float )( animation._animationTicksDuration ) );

  animateNodes( animation, animationTimeLooped, _rootNode, identityMatrix );
}

void sasSkeletonHierarchy::animateNodes( const sasAnimation& animation, float animationTime, sasSkeletonNode* node, smMat44& parentTransform )
{
  const sasAnimationChannel* channel = animation.findChannel( node->_name );

  if( channel != NULL )
  {
    smVec3 scaling = channel->computeInterpolatedScaling( animationTime );
    smVec3 translation = channel->computeInterpolatedTranslation( animationTime );
    aiQuaternion rotation = channel->computeInterpolatedRotation( animationTime );

    smMat44 localTransformMtx;
    aiMatrix4x4 aiMat( rotation.GetMatrix() );
    aiMat.a1 *= scaling.X; aiMat.b1 *= scaling.X; aiMat.c1 *= scaling.X;
    aiMat.a2 *= scaling.Y; aiMat.b2 *= scaling.Y; aiMat.c2 *= scaling.Y;
    aiMat.a3 *= scaling.Z; aiMat.b3 *= scaling.Z; aiMat.c3 *= scaling.Z;
    aiMat.a4 = translation.X; aiMat.b4 = translation.Y; aiMat.c4 = translation.Z;
    localTransformMtx.set( ( float* ) ( &aiMat ) );

    node->_animatedTransform = localTransformMtx;
  }
  else
  {
    node->_animatedTransform = node->_transform;
  }

  smMul( parentTransform, node->_animatedTransform, node->_animatedTransform );
  
  for( unsigned int i = 0 ; i < node->_numChildren; i++ ) 
  {
    animateNodes( animation, animationTime, node->_children[ i ], node->_animatedTransform );
  }
}


void sasSkeleton::applyAnimationFromSkeletonHierarchy()
{
  for( unsigned int boneIndex = 0; boneIndex < _numBones; boneIndex++ )
  {
    for( unsigned int nodeIndex = 0; nodeIndex < _skeletonHierarchy->_numNodes; nodeIndex++ )
    {
      if( _boneIdArray[ boneIndex ] == _skeletonHierarchy->_skeletonNodes[ nodeIndex ]._name )
      {
        smMul( _skeletonHierarchy->_skeletonNodes[ nodeIndex ]._animatedTransform, _boneMatrices[ boneIndex ], _transformedMatrices[ boneIndex ] );
        _transformedMatrices[ boneIndex ].transpose();
        break;
      }
    }
  }
}

sasStringId sasSkeleton::getParentBoneId( sasStringId boneId ) const
{
  static const sasStringId invalidId = sasStringId::build( "invalid" );

  int boneIndex = findBoneIndex( boneId );
  if( boneIndex )
  {
    if( _parentBoneIdArray[ boneIndex ] != invalidId )
      return _parentBoneIdArray[ boneIndex ];
    else
    {
      for( unsigned int nodeIndex = 0; nodeIndex < _skeletonHierarchy->_numNodes; nodeIndex++ )
      {
        if( boneId == _skeletonHierarchy->_skeletonNodes[ nodeIndex ]._name )
        {
          //found the node, check for parent
          if( _skeletonHierarchy->_skeletonNodes[ nodeIndex ]._parent != NULL )
          {
            sasStringId r = _skeletonHierarchy->_skeletonNodes[ nodeIndex ]._parent->_name;
            _parentBoneIdArray[ boneIndex ] = r;
            return r;
          }
        }
      }
      return invalidId;
    }
  }
  else
    return invalidId;
}

int sasSkeleton::findBoneIndex( sasStringId boneId ) const
{
  for( unsigned int boneIndex = 0; boneIndex < _numBones; boneIndex++ )
  {
    if( _boneIdArray[ boneIndex ] == boneId )
      return boneIndex;
  }
  //sasASSERT( false ); 
  return -1;
}

bool sasSkeleton::findBoneIndexSafe( sasStringId boneId, uint32_t& index ) const
{
  for( unsigned int boneIndex = 0; boneIndex < _numBones; boneIndex++ )
  {
    if( _boneIdArray[ boneIndex ] == boneId )
    {
      index = boneIndex;
      return true;
    }
  }

  index = -1;
  return false;
}

smVec3 sasSkeleton::getBoneWorldPosition( sasStringId boneId ) const
{
  int boneIndex = findBoneIndex( boneId );
  smVec4 bonePos( 0.f, 0.f, 0.f, 1.f );

  if( boneIndex >= 0 )
  {
    smMat44 xform = _boneMatrices[ boneIndex ];
    smInverse( xform, xform );

    //bonePos now in tPose
    smMul( xform, bonePos, bonePos );

    //apply animation transform
    smMat44 xf2 = _transformedMatrices[ boneIndex ];
    xf2.transpose();
    smMul( xf2, bonePos, bonePos );
  }
  
  return bonePos;
}

bool sasSkeleton::getBoneWorldPositionSafe( sasStringId boneId, smVec3& pos ) const
{
  uint32_t boneIndex = 0; 
  if( !findBoneIndexSafe( boneId, boneIndex ) )
    return false;

  smVec4 bonePos( 0.f, 0.f, 0.f, 1.f );

  smMat44 xform = _boneMatrices[ boneIndex ];
  smInverse( xform, xform );

  //bonePos now in tPose
  smMul( xform, bonePos, bonePos );

  //apply animation transform
  smMat44 xf2 = _transformedMatrices[ boneIndex ];
  xf2.transpose();
  smMul( xf2, bonePos, bonePos );

  pos = bonePos;
  return true;
}

bool sasSkeleton::serialize( sasVirtualFile& vFile ) const
{
  vFile.write( kVersion );

  vFile.writeStr( _skeletonId.string() );
  vFile.write( _numBones );

  for( uint32_t i = 0; i < _numBones; i++ )
  {
    vFile.writeStr( _boneIdArray[ i ].string() );
  }

  for( uint32_t i = 0; i < _numBones; i++ )
  {
    vFile.writeStr( _parentBoneIdArray[ i ].string() );
  }

  for( uint32_t i = 0; i < _numBones; i++ )
  {
    for( uint32_t j = 0; j < 4; j++ )
      vFile.write( _boneMatrices[ i ].L0[ j ] );

    for( uint32_t j = 0; j < 4; j++ )
      vFile.write( _boneMatrices[ i ].L1[ j ] );

    for( uint32_t j = 0; j < 4; j++ )
      vFile.write( _boneMatrices[ i ].L2[ j ] );

    for( uint32_t j = 0; j < 4; j++ )
      vFile.write( _boneMatrices[ i ].L3[ j ] );
  }

  return true;
}

bool sasSkeletonHierarchy::serialize( sasVirtualFile& vFile ) const
{
  sasASSERT( _rootNode != nullptr );
  vFile.write( findNodeIndex( *_rootNode ) );
  vFile.write( _numNodes );

  for( uint32_t i = 0; i < _numNodes; i++ )
  {
    _skeletonNodes[ i ].serialize( vFile, *this );
  }

  return true;
}

uint32_t sasSkeletonHierarchy::findNodeIndex( const sasSkeletonNode& node ) const
{
  for( unsigned int i = 0; i < _numNodes; i++ )
  {
    if( _skeletonNodes[ i ]._name == node._name )
    {
      return i;
    }
  }

  sasASSERT( false );
  return 0xffffffff;
}

bool sasSkeletonNode::serialize( sasVirtualFile& vFile, const sasSkeletonHierarchy& parentSkeletonHierarchy ) const
{
  vFile.writeStr( _name.string() );
  vFile.write( _boneIndex );
  vFile.write( ( _parent == nullptr ) ? 0xffffffff : parentSkeletonHierarchy.findNodeIndex( *_parent ) );
  vFile.write( _numChildren );
  for( uint32_t c = 0; c < _numChildren; c++ )
  {
    uint32_t childIndex = ( _children[ c ] != nullptr ) ? parentSkeletonHierarchy.findNodeIndex( *( _children[ c ] ) ) : 0xffffffff;
    vFile.write( childIndex );
  }

  for( uint32_t j = 0; j < 4; j++ )
    vFile.write( _transform.L0[ j ] );
  for( uint32_t j = 0; j < 4; j++ )
    vFile.write( _transform.L1[ j ] );
  for( uint32_t j = 0; j < 4; j++ )
    vFile.write( _transform.L2[ j ] );
  for( uint32_t j = 0; j < 4; j++ )
    vFile.write( _transform.L3[ j ] );

  return true;
}



