#pragma once

#include "render/sas_rendertypes.h"

class sasDrawList;
class sasConstantBuffer;
class sasRenderTarget;
class sasTexture;
class sasDepthStencilTarget;
class sasStructuredBuffer;
class sasVertexBuffer;
class sasIndexBuffer;
class sasDevice;

enum sasRenderMode
{
  sasRenderMode_default = 0, 
  sasRenderMode_BoundingSpheres,  
  sasRenderMode_Wireframe,
};

enum sasShaderStages
{
  sasStageVertexShader   = 0x01,
  sasStageHullShader     = 0x02,
  sasStageDomainShader   = 0x04,
  sasStageGeometryShader = 0x08,
  sasStagePixelShader    = 0x10
};

enum sasRenderListFlags
{
  sasRenderList_GBufferPass = 0,
  sasRenderList_ShadowPass = 0x01,
  sasRenderList_CubemapPass = 0x02,
};

#define NUM_SHADER_STAGES 5
#define MAX_NUM_SAMPLERS 16
#define MAX_NUM_SRV_TEXTURES 16
#define MAX_NUM_SRV_RTS 8
#define MAX_NUM_SRV_DSTS 4
#define MAX_NUM_SRV_STRUCTBUFFERS 4

struct sasSamplerStateBlob
{
  sasSamplerState       _samplerState;
  sasDeviceUnit::Enum   _stage;
  unsigned int          _index;
};

struct sasTextureBlob
{
  sasTexture*           _texture;
  sasDeviceUnit::Enum   _stage;
  unsigned int          _index;
};

struct sasRenderTargetBlob
{
  sasRenderTarget*      _rt;
  sasDeviceUnit::Enum   _stage;
  unsigned int          _index;
};

struct sasDepthStencilTargetBlob
{
  sasDepthStencilTarget*  _dst;
  sasDeviceUnit::Enum     _stage;
  unsigned int            _index;
};

struct sasStructuredBufferBlob
{
  sasStructuredBuffer*  _sb;
  sasDeviceUnit::Enum   _stage;
  unsigned int          _index;
};

struct sasModelRenderBlob
{
  sasShaderID                           _shaderID;
  unsigned int                          _stageMask;
  sasVertexBuffer*                      _vb;
  sasIndexBuffer*                       _ib;
  sasSamplerStateBlob                   _samplers[ MAX_NUM_SAMPLERS ];
  unsigned int                          _numSamplers;
  sasTextureBlob                        _srvTextures[ MAX_NUM_SRV_TEXTURES ];
  unsigned int                          _numSRVTextures;
  sasRenderTargetBlob                   _srvRTs[ MAX_NUM_SRV_RTS ];
  unsigned int                          _numSRVRTs;
  sasDepthStencilTargetBlob             _srvDSTs[ MAX_NUM_SRV_DSTS ];
  unsigned int                          _numSRVDSTs;
  sasStructuredBufferBlob               _srvStructuredBuffers[ MAX_NUM_SRV_STRUCTBUFFERS ];
  unsigned int                          _numSRVStructBuffers;
  sasPerInstanceConstants               _constants;
  sasRasterizerState                    _rasterizerState;
  sasDepthStencilState                  _depthState;
  sasBlendState                         _blendState;
  sasPrimTopology                       _topology;
  unsigned int                          _stencilRef;
  unsigned int                          _numPrimitives;
  unsigned int                          _instanceCount;
  float                                 _distToCamera;
  uint64_t                              _sortKey;

  sasModelRenderBlob()
  {
    _numSamplers = 0;
    _numSRVDSTs = 0;
    _numSRVRTs = 0;
    _numSRVStructBuffers = 0;
    _numSRVTextures = 0;
    _stageMask = 0;
    _instanceCount = 1;
  }

  void clear()
  {
    _numSamplers = 0;
    _numSRVDSTs = 0;
    _numSRVRTs = 0;
    _numSRVStructBuffers = 0;
    _numSRVTextures = 0;
    _stageMask = 0;
    _stencilRef = 0;
    _instanceCount = 1;
  }

  void generateSortKey( sasMaterialType::Enum type );

  //sasModelRenderBlob& operator =( const sasModelRenderBlob &rhs );
  void pushBlob( sasSamplerStateBlob& blob );
  void pushBlob( sasTextureBlob& blob );
  void pushBlob( sasStructuredBufferBlob& blob );
  void pushBlob( sasRenderTargetBlob& blob );
  void pushBlob( sasDepthStencilTargetBlob& blob );
};

class sasRenderList
{
  sasNO_COPY( sasRenderList );
  sasMEM_OP( sasRenderList );

public:
  void processList( sasDrawList* drawlist, unsigned int shaderFlagMask, smMat44* matProjView, const smVec3& cameraPos, sasRenderMode renderMode = sasRenderMode_default );
  void renderList( sasConstantBuffer* constantBuffer );

  static void renderDrawListImmediate( sasDrawList* drawlist, unsigned int shaderFlagMask, smMat44* matProjView, sasConstantBuffer* constantBuffer, sasRenderMode renderMode = sasRenderMode_default );

  sasRenderList();
  ~sasRenderList();

private:
  void clearList();
  void setSRVs( sasModelRenderBlob& mrb, sasDevice* device );

  static const int kMaxRenderBlobs = 2048;
  sasModelRenderBlob  _renderBlobs[ kMaxRenderBlobs ];
  unsigned int        _numRenderBlobs;
};
