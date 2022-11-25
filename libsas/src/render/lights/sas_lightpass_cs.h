#pragma once

#include "sas_lightpass.h"

class sasFrustum;

class sasLightPass_cs : public sasLightPass
{
public:
  sasLightPass_cs();
  virtual ~sasLightPass_cs();

  virtual void updateLightData( const sasFrustum& frustum );

  virtual void performLightPass( sasRenderTarget* lightBuffer, sasRenderTarget* specBuffer, sasRenderTarget* normalBuffer, sasDepthStencilTarget* depthBuffer, 
                                  sasRenderTarget* ssaoBuffer, sasRenderTarget* shadowBuffer );

  virtual void generateVolumetricLightBuffer( sasRenderTarget* volumetricLightBuffer );

  void generateLightListPerTile( uint32_t tileSize, float resX, float resY, const sasFrustum& frustum, const smMat44& viewProj, const smMat44& invProj );
  
private:
  sasStructuredBuffer*  _pointLightArrayBuffer;
  sasStructuredBuffer*  _spotLightArrayBuffer;

  std::vector< uint32_t > _culledTileLightIndices;
  std::vector< uint32_t > _tileStartIndices;
  std::vector< uint32_t > _tileNumIndices;
};
