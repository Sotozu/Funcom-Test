#include "sas_pch.h"
#include "sas_gpuprofiler.h"
#include "render/sas_gputimer.h"
#include "sas_device.h"

sasGpuProfiler::sasGpuProfiler()
{
  _currentFrame = 0;
  _enableGpuProfiling = false;
  _timingValuesCount = 0;

  // shadow debug bar
  _twGpuTimingsBar = TwNewBar( "GpuTimings" );
  TwDefine(" GpuTimings position='230 350' ");
  TwDefine(" GpuTimings visible=false "); 
}

sasGpuProfiler::~sasGpuProfiler()
{
}

void sasGpuProfiler::startGpuTimer( std::string& blockName )
{
  if( !_enableGpuProfiling )
    return;

  sasGPUTimingData& timing = _timings[ blockName ];
  if(  timing._timers[ _currentFrame ] == NULL )
  {
    for( unsigned int i = 0; i < kQueryLatency; i++ )
    {
      timing._timers[ i ] = sasNew sasGPUTimer();
    }
  }
  sasASSERT( timing._timers[ _currentFrame ]->state() == sasGPUTimer::sasGPUTimer_resolved );

  sasDevice::singletonPtr()->startTimer( *( timing._timers[ _currentFrame ] ) );
}

void sasGpuProfiler::endGpuTimer( std::string& blockName )
{
  if( !_enableGpuProfiling )
    return;

  sasGPUTimingData& timing = _timings[ blockName ];
  sasASSERT( timing._timers[ _currentFrame ]->state() == sasGPUTimer::sasGPUTimer_started );

  sasDevice::singletonPtr()->endTimer( *( timing._timers[ _currentFrame ] ) );
}

void sasGpuProfiler::processFrameTimings()
{
  if( !_enableGpuProfiling )
    return;

  TwRemoveAllVars( _twGpuTimingsBar );
  _currentFrame = ( _currentFrame + 1 ) % kQueryLatency;    


  // Iterate over all of the profiles
  int timingValuesCount = 0;
  GPUTimings::iterator it;
  for( it = _timings.begin(); it != _timings.end(); it++ )
  {
    sasGPUTimingData* timing = &( *it ).second;

    if( timing->_timers[ _currentFrame ]->state() != sasGPUTimer::sasGPUTimer_ended )
      continue;

    float profileTime = sasDevice::singletonPtr()->resolveTimer( *( timing->_timers[ _currentFrame ] ) );
    _timingValues[ timingValuesCount ] = profileTime;
    TwAddVarRO( _twGpuTimingsBar, it->first.c_str(), TW_TYPE_FLOAT, &_timingValues[ timingValuesCount ], "" );
    timingValuesCount++;
  }

}

sasGpuProfiler::sasGPUTimingData::sasGPUTimingData()
{
  for( unsigned int i = 0; i < kQueryLatency; i++ )
  {
    _timers[ i ] = NULL;
  }
}

sasGpuProfiler::sasGPUTimingData::~sasGPUTimingData()
{
  for( unsigned int i = 0; i < kQueryLatency; i++ )
  {
    if( _timers[ i ] )
      sasDelete _timers[ i ];
    _timers[ i ] = NULL;
  }
}

void sasGpuProfiler::setEnabled( bool enabled ) 
{ 
  if( enabled != _enableGpuProfiling )
  {
    _enableGpuProfiling = enabled; 
    if( enabled )
    {
      TwDefine(" GpuTimings visible=true ");
    }
    else
    {
      TwDefine(" GpuTimings visible=false "); 
    }
  }
}
