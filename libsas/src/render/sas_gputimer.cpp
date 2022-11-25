#include "sas_pch.h"
#include "sas_gputimer.h"
#include "sas_device_factory.h"

sasGPUTimer::sasGPUTimer()
  : _deviceObject( NULL )
  , _timerState( sasGPUTimer_resolved )
{
}

sasGPUTimer::~sasGPUTimer()
{
  if( _deviceObject && sasDeviceFactory::isValid() )
    sasDeviceFactory::singleton().releaseGPUTimer( _deviceObject ); 
}

sasDeviceObject* sasGPUTimer::deviceObject()
{
  if( !_deviceObject )
  {
    if( sasDeviceFactory::isValid() )
      _deviceObject = sasDeviceFactory::singleton().createGPUTimer( *this );  
  }
  return _deviceObject;
}