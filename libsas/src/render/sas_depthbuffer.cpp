#include "sas_pch.h"
#include "sas_depthbuffer.h"
#include "sas_device_factory.h"

sasDepthStencilTarget::sasDepthStencilTarget( const sasDepthStencilTargetDesc& dstDesc )
  : _deviceObject(0)
  , _desc( dstDesc )
{  
}

sasDepthStencilTarget::~sasDepthStencilTarget()
{
  if( _deviceObject && sasDeviceFactory::isValid() )
    sasDeviceFactory::singleton().releaseDepthStencilTarget(_deviceObject);
}

sasDeviceObject*  sasDepthStencilTarget::deviceObject()
{
  if( !_deviceObject )
  {
    if( sasDeviceFactory::isValid() )
      _deviceObject = sasDeviceFactory::singleton().createDepthStencilTarget( *this );  
  }
  return _deviceObject;
}
