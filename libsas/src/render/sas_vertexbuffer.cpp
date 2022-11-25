#include "sas_pch.h"
#include "sas_vertexbuffer.h"
#include "sas_device_factory.h"

sasVertexBuffer::sasVertexBuffer( sasVertexFormat fmt, size_t nVertices )
  : _deviceObject(0)
{
  Buffer& buffer = _buffers[0];
  buffer.format = fmt;
  buffer.nVertices = nVertices;
  buffer.sysBuffer = sasMalloc( fmt.size() * nVertices );  
}

sasVertexBuffer::~sasVertexBuffer()
{
  if( _deviceObject && sasDeviceFactory::isValid() )
    sasDeviceFactory::singleton().releaseVertexBuffer( _deviceObject ); 
  
  for( int i = 0; i < kMaxBuffers; ++i )
  {
    sasFree( _buffers[i].sysBuffer );
  }
}

void sasVertexBuffer::addBuffer( sasVertexFormat fmt, size_t nVertices, bool perInstanceData )
{
  // All buffers have to be added before the device object is created,
  // so the formats get set up correctly.

  sasASSERT(_deviceObject == NULL);
  for( int i = 0; i < kMaxBuffers; ++i )
  {
    Buffer& buffer = _buffers[i];
    if( buffer.sysBuffer == NULL )
    {
      buffer.format = fmt;
      buffer.nVertices = nVertices;
      buffer.sysBuffer = sasMalloc( fmt.size() * nVertices );
      buffer.perInstanceData = perInstanceData;
      return;
    }
  }
  
  sasASSERT(!"No space for more buffers.");
}
  
sasDeviceObject* sasVertexBuffer::deviceObject()
{
  if( !_deviceObject )
  {
    if( sasDeviceFactory::isValid() )
      _deviceObject = sasDeviceFactory::singleton().createVertexBuffer( *this );  
  }
  return _deviceObject;
}

size_t sasVertexBuffer::size(size_t buffer) const
{
  return _buffers[buffer].format.size() * _buffers[buffer].nVertices;
}

void* sasVertexBuffer::map( bool readonly, size_t buffer )
{
  return _buffers[buffer].sysBuffer;
}

void sasVertexBuffer::unmap()
{
}

