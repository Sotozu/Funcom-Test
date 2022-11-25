#include "sas_pch.h"
#include "sas_constantbuffer.h"
#include "sas_device_factory.h"
#include "sas_device.h"

sasConstantBuffer::sasConstantBuffer( size_t size )
  : _deviceObject(0),
    _size( size ),
    _dirty( 0 )
{
  _sysBuffer = sasMalloc( _size );
}

sasConstantBuffer::~sasConstantBuffer()
{
  if( _deviceObject && sasDeviceFactory::isValid() )
    sasDeviceFactory::singleton().releaseConstantBuffer( _deviceObject );  
  sasFree( _sysBuffer );
}

sasDeviceObject* sasConstantBuffer::deviceObject()
{
  if( sasDeviceFactory::isValid() )
  {
    if( !_deviceObject )
    {    
      _deviceObject = sasDeviceFactory::singleton().createConstantBuffer( *this );
    }    
  }  
  return _deviceObject;
}

void* sasConstantBuffer::map()
{
  _dirty = 1;
  return _sysBuffer;
}

void  sasConstantBuffer::unmap()
{
}
