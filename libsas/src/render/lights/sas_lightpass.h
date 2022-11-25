#pragma once

#include "utils/sas_singleton.h"

class sasRenderTarget;
class sasDepthStencilTarget;
class sasStructuredBuffer;
class sasConstantBuffer;
class sasFrustum;

class sasLightPass : public sasSingleton< sasLightPass >
{
  sasNO_COPY( sasLightPass );
  sasMEM_OP( sasLightPass );

public:
  sasLightPass();
  virtual ~sasLightPass();
  
  virtual void updateLightData( const sasFrustum& frustum );
  virtual void performLightPass( sasRenderTarget* lightBuffer, sasRenderTarget* specBuffer, sasRenderTarget* normalBuffer, sasDepthStencilTarget* depthBuffer, 
                                  sasRenderTarget* ssaoBuffer, sasRenderTarget* shadowBuffer ) = 0;
  virtual void generateVolumetricLightBuffer( sasRenderTarget* volumetricLightBuffer ) = 0;

protected:
  static const unsigned int kMaxPointLights = 3000;
  static const unsigned int kMaxSpotLights = 2000;

  sasConstantBuffer*    _constantBuffer;
};
