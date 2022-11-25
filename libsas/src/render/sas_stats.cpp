#include "sas_pch.h"
#include "sas_stats.h"
#include "utils/sas_timer.h"
#include "utils/sas_camera.h"
#include "utils/sas_cameramng.h"

sasRenderStats::sasRenderStats()
  : _twBar( 0 )
  , _fps( 0 )
  , _pointLightCount( 0 )
  , _spotLightCount( 0 )
  , _drawCount( 0 )
  , _stateChanges( 0 )
  , _cameraPos( 0.f, 0.f, 0.f )
{
  _twBar = TwNewBar( "RenderStats" );
  TwAddVarRO( _twBar, "FPS", TW_TYPE_FLOAT, &_fps, "" );
  TwAddVarRO( _twBar, "State Changes", TW_TYPE_UINT32, &_stateChanges, "" );
  TwAddVarRO( _twBar, "Draw Call", TW_TYPE_UINT32, &_drawCount, "" );
  TwAddVarRO( _twBar, "Camera Pos x", TW_TYPE_FLOAT, &_cameraPos.X, "" );
  TwAddVarRO( _twBar, "Camera Pos y", TW_TYPE_FLOAT, &_cameraPos.Y, "" );
  TwAddVarRO( _twBar, "Camera Pos z", TW_TYPE_FLOAT, &_cameraPos.Z, "" );
  TwAddVarRO( _twBar, "Point lights", TW_TYPE_UINT32, &_pointLightCount, "" );
  TwAddVarRO( _twBar, "Spot lights", TW_TYPE_UINT32, &_spotLightCount, "" );

  TwDefine(" RenderStats position='20 20' ");
}

void sasRenderStats::reset()
{
  _fps = sasTimer::singleton().getFPS();
  sasCamera* camera = sasCameraMng::singletonPtr()->getCurrentCamera();
  if( camera )
  {
    _cameraPos = camera->position();
  }
  _drawCount = _stateChanges = 0;
}

void sasRenderStats::incStateChanges( size_t inc )
{
  _stateChanges += inc;
}

void sasRenderStats::incDrawCount( size_t inc )
{
  _drawCount += inc;
}

void sasRenderStats::setPointLightCount( size_t numLights )
{
  _pointLightCount = numLights;
}

void sasRenderStats::setSpotLightCount( size_t numLights )
{
  _spotLightCount = numLights;
}

