#pragma once
#include "utils/sas_singleton.h"

class sasRenderTarget;
class sasDepthStencilTarget;
class sasConstantBuffer;

class sasCompositePass : public sasSingleton< sasCompositePass >
{
  sasNO_COPY( sasCompositePass );
  sasMEM_OP( sasCompositePass );

public:
  sasCompositePass();
  ~sasCompositePass();

  void applyCompositePass( sasRenderTarget* diffuseBuffer, sasRenderTarget* lightBuffer, sasRenderTarget* specBuffer, sasRenderTarget* godraysBuffer, 
                            sasDepthStencilTarget* depthBuffer, sasRenderTarget* ouput );

private:
  sasConstantBuffer* _cb;
};
