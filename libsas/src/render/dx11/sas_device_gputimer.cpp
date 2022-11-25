#include "sas_pch.h"
#include "sas_device_gputimer.h"
#include "sas_device_common.h"

sasDeviceGPUTimer::sasDeviceGPUTimer( ID3D11Query* disjointQuery, ID3D11Query* startQuery, ID3D11Query* endQuery )
  : sasDeviceObject( sasDeviceObjectType::GPUTimer )
  , _startQuery( startQuery )
  , _endQuery( endQuery )
  , _disjointQuery( disjointQuery )
{
}

sasDeviceGPUTimer::~sasDeviceGPUTimer()
{
  DX_RELEASE( _disjointQuery );
  DX_RELEASE( _startQuery );
  DX_RELEASE( _endQuery );
}
