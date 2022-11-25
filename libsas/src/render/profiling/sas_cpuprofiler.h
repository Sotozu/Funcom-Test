#pragma once

#include "utils/sas_singleton.h"

#define INSERT_CPU_TIMER( name, strName ) sasCpuProfilerEvent cpuEvent##name( strName );

class sasCpuProfiler : public sasSingleton< sasCpuProfiler >
{
  sasNO_COPY( sasCpuProfiler );
  sasMEM_OP( sasCpuProfiler );

public:
  sasCpuProfiler();
  ~sasCpuProfiler();

  void startCpuTimer( std::string& blockName );
  void endCpuTimer( std::string& blockName );

  void processFrameTimings();

  void setEnabled( bool enabled );

private:
  struct sasCPUTimingData
  {
    float startTime;
    float endTime;
    float totalTime;
  };
  typedef std::map< std::string, sasCPUTimingData > CPUTimings;

  CPUTimings  _timings;
  
  TwBar*  _twCpuTimingsBar;
  float _timingValues[ 100 ];
  unsigned int _timingValuesCount;
  bool  _enableCpuProfiling;
};


struct sasCpuProfilerEvent
{
  std::string _eventName;

  sasCpuProfilerEvent( const char* eventName )
  {
    _eventName = eventName;
    sasCpuProfiler::singletonPtr()->startCpuTimer( _eventName );
  }

  ~sasCpuProfilerEvent()
  {
    sasCpuProfiler::singletonPtr()->endCpuTimer( _eventName );
  }
};