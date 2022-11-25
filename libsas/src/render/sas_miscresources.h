#pragma once

#include "utils/sas_singleton.h"
#include "sas_stereo.h"

class sasVertexBuffer;
class sasConstantBuffer;
class sasTexture;
class sasIndexBuffer;
class sasTextureResource;
class sasStructuredBuffer;
struct sasVertexFormat;

class sasMiscResources : public sasSingleton<sasMiscResources>
{
  sasNO_COPY( sasMiscResources );
  sasMEM_OP( sasMiscResources );

public:
  sasMiscResources();
  ~sasMiscResources();

  void loadMiscTextures();

  sasVertexBuffer* getFullScreenQuadVB() const { return _fullScreenQuadVB; }
  sasVertexBuffer* getUnitCubeVB() const { return _unitCubeVB; }
  sasIndexBuffer*  getUnitCubeIB() const { return _UnitCubeIB; }
  sasTexture*      getNoiseTexture() const;
  sasVertexBuffer* getUnitSphereVB() const { return _unitSphereVB; }
  sasIndexBuffer*  getUnitSphereIB() const { return _unitSphereIB; }
  sasStructuredBuffer* getNoiseStructuredBuffer() const { return _noiseSB; }

  void updateGlobalShaderConstants( float backbufferWidth, float backbufferHeight, smMat44* view = NULL, smMat44* proj = NULL, bool initialFrameSet = false, sasStereoView::Enum eye = sasStereoView::Mono );

  sasTexture* getMissingTexture() const;
  sasTexture* getSelectedTexture()  const;

  void createUnitSphere( sasVertexBuffer** vb, sasIndexBuffer** ib, uint32_t ringCount, uint32_t segmentCount, const sasVertexFormat& vertexFmt );

private:
  void createFullScreenQuadVB();
  void createUnitCube();
  void createUnitSphere();
  void createNoiseSB();

private:
  sasVertexBuffer*    _fullScreenQuadVB;
  sasVertexBuffer*    _unitCubeVB;
  sasIndexBuffer*     _UnitCubeIB;
  sasVertexBuffer*    _unitSphereVB;
  sasIndexBuffer*     _unitSphereIB;
  sasConstantBuffer*  _globalConstants;
  sasTexture*         _noiseTexture;
  sasTextureResource* _missingTexture;
  sasTextureResource* _selectedTexture;
  sasStructuredBuffer*_noiseSB;
};
