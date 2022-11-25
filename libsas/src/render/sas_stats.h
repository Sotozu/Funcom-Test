#pragma once

#include "utils/sas_singleton.h"

class sasRenderStats : public sasSingleton<sasRenderStats>
{
  sasNO_COPY( sasRenderStats );
  sasMEM_OP( sasRenderStats );

public: //! ctor/dtor

  sasRenderStats();
  ~sasRenderStats(){ TwDeleteBar(_twBar); }

public: //! methods

  void reset();
  void incStateChanges( size_t inc );
  void incDrawCount( size_t inc = 1 );
  void setPointLightCount( size_t numLights );
  void setSpotLightCount( size_t numLights );

private: //! members

  TwBar*  _twBar;

  float   _fps;
  size_t  _pointLightCount;
  size_t  _spotLightCount;
  size_t  _drawCount;
  size_t  _stateChanges;
  smVec3  _cameraPos;


};
