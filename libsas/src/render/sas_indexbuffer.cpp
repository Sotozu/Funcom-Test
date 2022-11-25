#include "sas_pch.h"
#include "sas_indexbuffer.h"
#include "sas_device_factory.h"

sasIndexBuffer::sasIndexBuffer( size_t nIndices ) :
  _deviceObject( 0 ),
  _nIndices( nIndices )
{
  _sysBuffer = sasMalloc( size() );  
}

sasIndexBuffer::~sasIndexBuffer()
{  
  if( _deviceObject && sasDeviceFactory::isValid() )
    sasDeviceFactory::singleton().releaseIndexBuffer( _deviceObject );  
  sasFree( _sysBuffer );
}

sasDeviceObject* sasIndexBuffer::deviceObject()
{
  if( !_deviceObject )
  {
    if( sasDeviceFactory::isValid() )
      _deviceObject = sasDeviceFactory::singleton().createIndexBuffer( *this );  
  }
  return _deviceObject;
}

unsigned short* sasIndexBuffer::map()
{
  return (unsigned short*)_sysBuffer;
}

void sasIndexBuffer::unmap()
{
}
