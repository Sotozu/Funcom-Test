#pragma once

#include "utils/sas_singleton.h"

class sasDrawList;
class sasRenderList;
class sasDepthStencilTarget;
class sasConstantBuffer;
class sasDevice;
class sasRenderTarget;
class sasFrustumPoints;
class sasSpotShadowPool;

class sasShadowMng : public sasSingleton< sasShadowMng >
{
  sasNO_COPY( sasShadowMng );
  sasMEM_OP( sasShadowMng );

public:
  sasShadowMng();
  ~sasShadowMng();

  void frameInitialize();
  void frameEnd();

  void updateMatrices( sasCamera& sceneCam, const smVec3& lightDir, float vpWidth, float vpHeight );

  void renderShadowMaps( sasDevice* device, sasConstantBuffer* constantBuffer );
  void viewMatrix( smMat44* viewMtx, unsigned int cascade_index ) { *viewMtx = _lightView[ cascade_index ]; }
  void projMatrix( smMat44* projMtx, unsigned int cascade_index ) { *projMtx = _lightProj[ cascade_index ]; }
  void projViewMatrix( smMat44* projViewMtx, unsigned int cascade_index ){ *projViewMtx = _lightProjView[ cascade_index ]; }
  sasDepthStencilTarget* shadowMap( unsigned int cascade_index ) { return _shadowmap[ cascade_index ]; }

  void computeScreenSpaceShadows( sasRenderTarget* normalBuffer, sasDepthStencilTarget* depthBuffer );
  sasRenderTarget* screenSpaceShadowMap() { return _screenSpaceShadowMap; }

  void computeGodrays( sasDepthStencilTarget* depthBuffer, sasRenderTarget* volLightRT );
  sasRenderTarget* godRayBuffer() { return _godraysBuffer[ 1 ]; }

  void setCascadeRanges( const smVec4& ranges );
  void setGodraysIntensity( float intensity );

  void clearScreenSpaceShadowMap();
  void clearGodrayBuffer();

private:
  void updateMatricesForCascade( sasFrustumPoints& cascadeFrustum, sasCamera& sceneCam, const smVec3& lightDir, float vpWidth, float vpHeight, smMat44& view, smMat44& proj, smMat44& projView );
  void blurGodrays();

private:
  smVec2  _shadowMapResolution;
  smMat44 _lightView[ 3 ];
  smMat44 _lightProj[ 3 ];
  smMat44 _lightProjView[ 3 ];
  sasDrawList* _shadowList;
  sasRenderList* _compiledShadowList;
  sasDepthStencilTarget* _shadowmap[ 3 ];
  sasRenderTarget* _screenSpaceShadowMap;
  sasRenderTarget* _godraysBuffer[ 2 ];
  sasConstantBuffer* _shadowConstants;
  sasConstantBuffer* _godraysBlurConstants;
  float _cascadeDistances[ 3 ];
  float _godraysIntensity;
  smVec3 _lightDirection;
  smVec2 _godrayMapResolution;

  //local sm pools
  sasSpotShadowPool* _spotShadowMapPool;
};
