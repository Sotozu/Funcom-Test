#include "sas_pch.h"
#include "sas_indirectargsbuffer.h"
#include "sas_device_factory.h"
#include "sas_device.h"

sasIndirectArgsBuffer::sasIndirectArgsBuffer( size_t size, unsigned int* initialData, bool staging )
  : _deviceObject(0),
    _size( size ),
    _dirty( 0 ),
    _staging( staging )
{
  _sysBuffer = sasMalloc( _size );
  memcpy( _sysBuffer, static_cast< void* >( initialData ), size );
}

sasIndirectArgsBuffer::~sasIndirectArgsBuffer()
{
  if( _deviceObject && sasDeviceFactory::isValid() )
    sasDeviceFactory::singleton().releaseIndirectArgsBuffer( _deviceObject );  
  sasFree( _sysBuffer );
}

sasDeviceObject* sasIndirectArgsBuffer::deviceObject()
{
  if( sasDeviceFactory::isValid() )
  {
    if( !_deviceObject )
    {    
      _deviceObject = sasDeviceFactory::singleton().createIndirectArgsBuffer( *this );
    }    
  }  
  return _deviceObject;
}


