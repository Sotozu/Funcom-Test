#pragma once 

#include "sm_matrix.h"

class sasGeometry;
class sasStructuredBuffer;
class sasSkeleton;
class sasModelInstance;

class sasStagingInstance
{
  sasNO_COPY( sasStagingInstance );
  sasMEM_OP( sasStagingInstance );

public:
  //create staging instance, not present in engine yet
  sasStagingInstance( sasGeometry* geometry );
  ~sasStagingInstance();

public:
  const smMat44& transform() const { return _transform; }

  void setTransform( const smMat44& m )   
  { 
    _transform = m; 
  }

  sasGeometry* geometry() const { return _geometry; } 
  const smVec4* lsBoundingSphere();

private:
  smMat44       _transform;
  sasGeometry*  _geometry;
};

class sasInstance
{
  sasMEM_OP( sasInstance );

public:
  //create actual instance from staging instance
  sasInstance( sasStagingInstance& instance, unsigned int uid );

  ~sasInstance();

public:
  typedef std::vector< sasInstance* > Instances;
  typedef Instances::iterator         Iterator;

  static Iterator begin() { return _instances.begin(); }
  static Iterator end()   { return _instances.end(); }

public:
  const smMat44& transform() const              { return _transform; }
  const smMat44& worldMatrix() const            { return _worldMatrix; }
  const smMat44& lastFrameWorldMatrix() const   { return _previousFrameWorldMatrix; }
  const smVec4&  tintColour() const             { return _tintColour; }  

  void setTransform( const smMat44& m );
  void setInstanceTransform( const smMat44& m ); 
  void setTintColour( const smVec4& tintCol )     { _tintColour = tintCol; }

  //deprecated method, do not use
  void setFrameTransform( const smMat44& m );

  void updateInstancePostFrame();

  sasGeometry*  geometry()                { return _geometry; } 
  const smVec4* lsBoundingSphere();
  const smVec4* wsBoundingSphere()        { return &_wsBoundingShphere; }

  sasStructuredBuffer*  skinningMatricesBuffer( size_t subMeshIndex ) const     { return _skinningMatrices[ subMeshIndex ]; }
  void                  updateSkinningMatrices( size_t subMeshIndex, const sasSkeleton* skeleton );

  unsigned int  uid() const { return _uid; }
  float         uidFloat() const { return _uidFloat; }

  void                renderSkeleton() const;
  bool                getBoneWorldPosition( sasStringId boneId, smVec3& pos ) const;
  sasModelInstance*   parentModelInstance() const                             { return _parentModelInstance; }
  void                setParentModelInstance( sasModelInstance* modelInst )   { _parentModelInstance = modelInst; }
  const sasSkeleton*  skeleton( unsigned int submeshIndex ) const;
  uint32_t            numSubmeshes() const;

  void setInstancedData( void* data, uint32_t size );
  void setInstanceCount( uint32_t numInstances ) { _numInstances = numInstances; }
  uint32_t instanceCount() const { return _numInstances; }
  sasStructuredBuffer* instancedData() const { return _instancedData; }

private:
  void updateLastFrameData();

private:
  static Instances _instances; 

  smMat44             _transform;
  smMat44             _instanceTranform;
  smMat44             _worldMatrix;
  smMat44             _previousFrameWorldMatrix;
  smVec4              _tintColour;
  sasGeometry*        _geometry;
  unsigned int        _uid;
  float               _uidFloat;

  std::vector< const sasSkeleton * > _skeletons;
  sasModelInstance*     _parentModelInstance;

  std::vector< sasStructuredBuffer* > _skinningMatrices;
  smVec4 _wsBoundingShphere;

  sasStructuredBuffer*  _instancedData;
  uint32_t              _numInstances;

};
