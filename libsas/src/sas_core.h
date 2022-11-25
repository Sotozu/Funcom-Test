#pragma once 

#include "sas_main.h"

class sasRenderMng;
class sasResourceMng;
class sasCameraMng;
class sasLightMng;
class sasContiguousMemPool;

class sasCore
{
  sasNO_COPY(sasCore);

public:
  sasCore( const sasConfig& config );
  ~sasCore();

public:
  void update();

private:

  sasRenderMng*     _renderMng;
  sasResourceMng*   _resourceMng;  
  sasLightMng*      _lightMng;
  sasCameraMng*     _cameraMng;  
  sasContiguousMemPool* _contiguousMemPool;
};