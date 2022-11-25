#pragma once

#include "utils/sas_singleton.h"

class sasGPUTimer;

#define INSERT_GPU_TIMER( name, strName ) sasGpuProfilerEvent gpuEvent##name( strName );

class sasGpuProfiler : public sasSingleton< sasGpuProfiler >
{
  sasNO_COPY( sasGpuProfiler );
  sasMEM_OP( sasGpuProfiler );

public:
  sasGpuProfiler();
  ~sasGpuProfiler();

  void startGpuTimer( std::string& blockName );
  void endGpuTimer( std::string& blockName );

  void processFrameTimings();

  void setEnabled( bool enabled );

private:
  static const UINT64 kQueryLatency = 5;
  struct sasGPUTimingData
  {
    sasGPUTimer* _timers[ kQueryLatency ];

    sasGPUTimingData();
    ~sasGPUTimingData();
  };
  typedef std::map< std::string, sasGPUTimingData > GPUTimings;

  GPUTimings  _timings;
  UINT64      _currentFrame;

  TwBar*  _twGpuTimingsBar;
  float _timingValues[ 100 ];
  unsigned int _timingValuesCount;
  bool  _enableGpuProfiling;
};


struct sasGpuProfilerEvent
{
  std::string _eventName;

  sasGpuProfilerEvent( const char* eventName )
  {
    _eventName = eventName;
    sasGpuProfiler::singletonPtr()->startGpuTimer( _eventName );
  }

  ~sasGpuProfilerEvent()
  {
    sasGpuProfiler::singletonPtr()->endGpuTimer( _eventName );
  }
};