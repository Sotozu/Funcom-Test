#pragma once

#include "utils/sas_singleton.h"
#include "sm_common.h"

class sasDrawList;
class sasRenderList;
class sasDepthStencilTarget;
class sasConstantBuffer;
class sasDevice;
class sasRenderTarget;
class sasSpotLight;
class sasCamera;
class sasFrustum;

smALIGN16_PRE class sasSpotShadowPoolEntry 
{
  sasNO_COPY( sasSpotShadowPoolEntry );
  sasMEM_OP( sasSpotShadowPoolEntry );

public:
  sasSpotShadowPoolEntry( size_t resolutionX, size_t resolutionY );
  ~sasSpotShadowPoolEntry();

  void                    setLight( sasSpotLight* light );

  void                    setupRenderPass();
  void                    renderShadowMap( sasDevice* device, sasConstantBuffer* constantBuffer );

  const smMat44&          projViewMtx() const   { return _projViewMtx; }
  sasDepthStencilTarget*  shadowMap() const     { return _shadowMap; }
  const smVec2&           resolution() const    { return _shadowMapResolution; }

  void                    stepPostFrame();

  bool                    _isInUse() const      { return ( _light != NULL ); }


private:
  smVec2                  _shadowMapResolution;
  sasDrawList*            _shadowList;
  sasRenderList*          _compiledShadowList;
  smMat44                 _viewMtx;
  smMat44                 _projMtx;
  smMat44                 _projViewMtx;
  sasDepthStencilTarget*  _shadowMap;
  sasSpotLight*           _light;
  sasConstantBuffer*      _shadowConstants;
  bool                    _needUpdate;

} smALIGN16_POST;

class sasSpotShadowPool : public sasSingleton< sasSpotShadowPool >
{
  sasNO_COPY( sasSpotShadowPool );
  sasMEM_OP( sasSpotShadowPool );

public:
  sasSpotShadowPool();
  ~sasSpotShadowPool();

  void registerSpotLight( sasSpotLight* light );
  void unregisterSpotLight( sasSpotLight* light );

  sasDepthStencilTarget* spotShadowMap( sasSpotLight* light ) const;

  void preRenderStep( const sasCamera& cam, const sasFrustum& frustum );
  void updateShadowMaps( sasDevice* device, sasConstantBuffer* constantBuffer );
  void stepPostFrame();

private:
  int findSpotLightEntryIndex( sasSpotLight* light ) const;

private:
  static const int kMaxShadowMapEntries = 4;
  sasSpotShadowPoolEntry* _spotShadowMapEntries[ kMaxShadowMapEntries ];

  typedef std::pair< sasSpotLight*, int > SMPoolEntry;
  std::vector< SMPoolEntry > _entryLinks;
  sasConstantBuffer* _shadowConstants;
};
