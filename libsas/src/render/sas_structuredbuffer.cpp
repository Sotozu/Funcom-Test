#include "sas_pch.h"
#include "sas_structuredbuffer.h"
#include "sas_device_factory.h"
#include "sas_device.h"

sasStructuredBuffer::sasStructuredBuffer( size_t size, size_t structuredByteStride, bool dynamic, bool bAppendConsume, bool bStaging )
  : _deviceObject(0),
    _size( size ),
    _dirty( 0 ),
    _dynamic( dynamic ),
    _structuredByteStride( structuredByteStride ),
    _appendConsume( bAppendConsume ),
    _staging( bStaging ),
    _sysBuffer( nullptr )
{
  if( dynamic )
  {
    _sysBuffer = sasMalloc( _size );
  }
}

sasStructuredBuffer::~sasStructuredBuffer()
{
  if( _deviceObject && sasDeviceFactory::isValid() )
    sasDeviceFactory::singleton().releaseStructuredBuffer( _deviceObject ); 

  if( _sysBuffer )
  {
    sasFree( _sysBuffer );
  }
}

sasDeviceObject* sasStructuredBuffer::deviceObject()
{
  if( sasDeviceFactory::isValid() )
  {
    if( !_deviceObject )
    {    
      _deviceObject = sasDeviceFactory::singleton().createStructuredBuffer( *this );
    }    
  }  
  return _deviceObject;
}

void* sasStructuredBuffer::map()
{
  sasASSERT( _dynamic == true );
  _dirty = 1;
  return _sysBuffer;
}

void  sasStructuredBuffer::unmap()
{
  sasASSERT( _dynamic == true );
}
