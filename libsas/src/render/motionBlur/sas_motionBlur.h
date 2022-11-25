#pragma once
#include "utils/sas_singleton.h"

class sasRenderTarget;
class sasDepthStencilTarget;
class sasStructuredBuffer;
class sasConstantBuffer;

class sasMotionBlur : public sasSingleton< sasMotionBlur >
{
  sasNO_COPY( sasMotionBlur );
  sasMEM_OP( sasMotionBlur );

public:
  sasMotionBlur();
  ~sasMotionBlur();

  void performMotionBlurPass( sasRenderTarget* colourBuffer, sasRenderTarget* velocityBuffer, sasRenderTarget* outputBuffer, sasDepthStencilTarget* depthBuffer );

private:
  sasConstantBuffer* _constantBuffer;
};
