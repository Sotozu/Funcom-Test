#pragma once

#include "utils/sas_singleton.h"
#include "render/sas_rendertypes.h"

class sasRenderTarget;
class sasDepthStencilTarget;
class sasCamera;
class sasDevice;
class sasVertexBuffer;
class sasFrustumPoints;
class sasConstantBuffer;
struct sasColor;
class sasModelInstance;

struct sasDebugSphere
{
  smVec3 _pos;
  float  _radius;
  smVec4 _col;
  sasDepthStencilState depthState;
  sasRasterizerState rasterState;
};

struct sasDebugLine
{
  smVec3 _posA;
  smVec3 _posB;
  smVec4 _col;
  sasDepthStencilState depthState;
};

struct sasDebugBox
{
  smAABB  _aabb;
  smMat44 _xform;
  smVec4  _col;
  sasDepthStencilState depthState;
  sasRasterizerState rasterState;
};

class sasDebugRender : public sasSingleton< sasDebugRender >
{
  sasNO_COPY( sasDebugRender );  
  sasMEM_OP( sasDebugRender );

public:
  sasDebugRender();
  ~sasDebugRender();

  void renderBox( const smAABB& aabb, const smMat44& xform, const smVec4& colour );
  void renderSphere( const smVec3& pos, float radius, const smVec4& colour );
  void renderLine( const smVec3& pos_a, const smVec3& pos_b, const sasColor& col, sasDepthStencilState depthState = sasDepthStencilState_LessEqual );
  void renderFrustum( const sasFrustumPoints& frustum );
  void renderVolumeTextureSlice( sasRenderTarget* volumeTexture, unsigned int slice );

  void queueRenderBox( const sasDebugBox& b );
  void queueRenderSphere( const sasDebugSphere& s );
  void queueRenderLine( const sasDebugLine& l );
  void queueRenderSkeleton( const sasStringId& modelInstId );
  void queueRenderBone( const sasStringId& modelInstId, const sasStringId& boneId );
  void flushQueuedDebugObjects( sasRenderTarget* rt, sasDepthStencilTarget* dst );

private:
  sasConstantBuffer* _lineCB;
  sasConstantBuffer* _volumeTexCB;
  sasConstantBuffer* _boxCB;

  static const uint32_t kMaxDebugSpheres = 4096;
  sasDebugSphere  _debugSphereQueue[ kMaxDebugSpheres ];
  uint32_t        _numDebugSpheres;

  static const uint32_t kMaxDebugBoxes = 4096;
  sasDebugBox     _debugBoxQueue[ kMaxDebugBoxes ];
  uint32_t        _numDebugBoxes;
  
  static const uint32_t kMaxDebugLines = 512;
  sasDebugLine    _debugLineQueue[ kMaxDebugLines ];
  uint32_t        _numDebugLines;

  static const uint32_t kMaxDebugSkeletons = 16;
  sasModelInstance* _debugSkeletonQueue[ kMaxDebugSkeletons ];
  uint32_t          _numDebugSkeletons;

  static const uint32_t kMaxDebugBones = 256;
  smVec4            _debugBoneQueue[ kMaxDebugBones ];
  uint32_t          _numDebugBones;
};



