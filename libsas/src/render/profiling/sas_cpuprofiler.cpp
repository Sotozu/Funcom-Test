#include "sas_pch.h"
#include "sas_cpuprofiler.h"
#include "utils/sas_timer.h"
#include "sas_device.h"

sasCpuProfiler::sasCpuProfiler()
{
  _enableCpuProfiling = false;
  _timingValuesCount = 0;

  // shadow debug bar
  _twCpuTimingsBar = TwNewBar( "CpuTimings" );
  TwDefine(" CpuTimings position='230 350' ");
  TwDefine(" CpuTimings visible=false "); 
}

sasCpuProfiler::~sasCpuProfiler()
{
}

void sasCpuProfiler::startCpuTimer( std::string& blockName )
{
  if( !_enableCpuProfiling )
    return;

  sasCPUTimingData& timing = _timings[ blockName ];
  timing.startTime = sasTimer::singletonPtr()->getExactTime();
}

void sasCpuProfiler::endCpuTimer( std::string& blockName )
{
  if( !_enableCpuProfiling )
    return;

  sasCPUTimingData& timing = _timings[ blockName ];
  timing.endTime = sasTimer::singletonPtr()->getExactTime();
  timing.totalTime = ( timing.endTime - timing.startTime ) * 1000.f;
}

void sasCpuProfiler::processFrameTimings()
{
  if( !_enableCpuProfiling )
    return;

  TwRemoveAllVars( _twCpuTimingsBar );

  // Iterate over all of the profiles
  int timingValuesCount = 0;
  CPUTimings::iterator it;
  for( it = _timings.begin(); it != _timings.end(); it++ )
  {
    sasCPUTimingData* timing = &( *it ).second;

    float profileTime = timing->totalTime;
    _timingValues[ timingValuesCount ] = profileTime;
    TwAddVarRO( _twCpuTimingsBar, it->first.c_str(), TW_TYPE_FLOAT, &_timingValues[ timingValuesCount ], "" );
    timingValuesCount++;
  }

}

void sasCpuProfiler::setEnabled( bool enabled ) 
{ 
  if( enabled != _enableCpuProfiling )
  {
    _enableCpuProfiling = enabled; 
    if( enabled )
    {
      TwDefine(" CpuTimings visible=true ");
    }
    else
    {
      TwDefine(" CpuTimings visible=false "); 
    }
  }
}
