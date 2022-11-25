#pragma once
#include "utils/sas_singleton.h"

class sasRenderTarget;
class sasDepthStencilTarget;
class sasStructuredBuffer;
class sasConstantBuffer;

class sasVolumetricLightMng : public sasSingleton< sasVolumetricLightMng >
{
  sasNO_COPY( sasVolumetricLightMng );
  sasMEM_OP( sasVolumetricLightMng );

public:
  sasVolumetricLightMng();
  ~sasVolumetricLightMng();

  void performVolumetricLightPass( sasRenderTarget* lightBuffer, sasDepthStencilTarget* depthBuffer );

private:
  sasConstantBuffer* _constantBuffer;
};
