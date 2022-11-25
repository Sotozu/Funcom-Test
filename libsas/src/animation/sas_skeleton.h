#pragma once 
#include "utils/sas_stringid.h"

class sasAnimation;
class sasVirtualFile;
struct sasBone
{
  sasStringId _boneId;
  smMat44 _offsetMatrix;
};

struct sasVertexInfluence
{
  unsigned int _vertexIndex;
  float _boneWeight;
};

struct sasBoneMapping
{
  sasStringId _boneId;
  unsigned int _boneIndex;
  std::vector< sasVertexInfluence > _vertexInfluences;
};

class sasSkeletonMapping
{
   sasMEM_OP( sasSkeletonMapping );
public:
  sasSkeletonMapping(){}
  ~sasSkeletonMapping(){}

  void findBoneMappingForVertex( unsigned int vertexIndex, int& boneIndex0, float& boneWeight0,
                                                            int& boneIndex1, float& boneWeight1,
                                                            int& boneIndex2, float& boneWeight2,
                                                            int& boneIndex3, float& boneWeight3 ) const;

  std::vector< sasBoneMapping > _boneMappings;
};

class sasSkeletonHierarchy;
struct sasSkeletonNode
{
  sasMEM_OP( sasSkeletonNode );
public:
  sasSkeletonNode();

  bool serialize( sasVirtualFile& vFile, const sasSkeletonHierarchy& parentSkeletonHierarchy ) const;

public:
  static const int kMaxNumChildrenNodes = 64;

  sasStringId _name;
  unsigned int _boneIndex;
  sasSkeletonNode* _parent;
  unsigned int _numChildren;
  sasSkeletonNode* _children[ kMaxNumChildrenNodes ];
  smMat44 _transform;
  smMat44 _animatedTransform;
};

class sasSkeletonHierarchy
{
  sasMEM_OP( sasSkeletonHierarchy );
public:
  sasSkeletonHierarchy();
  ~sasSkeletonHierarchy();

  void applyAnimation( const sasAnimation& animation, float animationTime );

  bool serialize( sasVirtualFile& vFile ) const;
  uint32_t findNodeIndex( const sasSkeletonNode& node ) const;

private:
  void animateNodes( const sasAnimation& animation, float animationTime, sasSkeletonNode* node, smMat44& parentTransform );

public:
  sasSkeletonNode* _rootNode;
  sasSkeletonNode* _skeletonNodes;
  unsigned int _numNodes;
};

class sasSkeleton
{
  sasMEM_OP( sasSkeleton );
public:
  sasSkeleton( unsigned int numBones );
  ~sasSkeleton() {}

  uint32_t    numBones() const                { return _numBones; }
  sasStringId boneId( uint32_t index ) const  { return _boneIdArray[ index ]; }

  int findBoneIndex( sasStringId boneId ) const;
  bool findBoneIndexSafe( sasStringId boneId, uint32_t& index ) const;
  void applyAnimationFromSkeletonHierarchy();

  smVec3 getBoneWorldPosition( sasStringId boneId ) const;
  bool getBoneWorldPositionSafe( sasStringId boneId, smVec3& pos ) const;
  sasStringId getParentBoneId( sasStringId boneId ) const;

  bool serialize( sasVirtualFile& vFile ) const;

public:
  static const unsigned int sMaxBones = 128;
  sasStringId _skeletonId;
  sasStringId _boneIdArray[ sMaxBones ];
  mutable sasStringId _parentBoneIdArray[ sMaxBones ];
  smMat44     _boneMatrices[ sMaxBones ];
  smMat44     _transformedMatrices[ sMaxBones ];
  unsigned int _numBones;
  sasSkeletonHierarchy* _skeletonHierarchy;

  static const unsigned int kVersion = 1;
};

