#pragma once

#include "sas_lightpass.h"

class sasVertexBuffer;
class sasFrustum;

class sasLightPass_ps : public sasLightPass
{
public:
    sasLightPass_ps();
    virtual ~sasLightPass_ps();

    virtual void updateLightData( const sasFrustum& frustum );
    virtual void performLightPass( sasRenderTarget* lightBuffer, sasRenderTarget* specBuffer, sasRenderTarget* normalBuffer, sasDepthStencilTarget* depthBuffer, 
                                    sasRenderTarget* ssaoBuffer, sasRenderTarget* shadowBuffer );
    virtual void generateVolumetricLightBuffer( sasRenderTarget* volumetricLightBuffer );

  private:
    
    struct PointlightConstantBuffer
    {
      smVec4 colour;
      smVec4 position;
      float radius;
    };

    sasConstantBuffer* _pointLightListBuffer;
    sasVertexBuffer* _lightArrayVB;
    size_t _numActiveLights;
};
