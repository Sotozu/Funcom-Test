#pragma once

#include "utils/sas_singleton.h"

class sasConstantBuffer;
class sasDevice;
class sasRenderTarget;

class sasPostAA : public sasSingleton< sasPostAA >
{
  sasNO_COPY( sasPostAA );
  sasMEM_OP( sasPostAA );

public:
  sasPostAA();
  ~sasPostAA();

  void computePostAA( sasRenderTarget* imageBuffer, sasRenderTarget* destBuffer );

private:
  sasConstantBuffer* _constants;
};
