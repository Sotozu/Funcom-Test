#pragma once

#include "sas_vertexformat.h"

class sasDeviceObject;

class sasVertexBuffer
{
  sasNO_COPY( sasVertexBuffer );
  sasMEM_OP( sasVertexBuffer );

public:
  static const int kMaxBuffers = 4;

  sasVertexBuffer( sasVertexFormat fmt, size_t nVertices );
  ~sasVertexBuffer();

public:
  void              addBuffer( sasVertexFormat fmt, size_t nVertices, bool perInstanceData );
  sasDeviceObject*  deviceObject();
  size_t            size(size_t buffer=0) const;
  sasVertexFormat   format(size_t buffer=0) const { return _buffers[buffer].format; }
  size_t            verticesCount(size_t buffer=0) const { return _buffers[buffer].nVertices; }
  void*             buffer(size_t buffer=0) const        { return _buffers[buffer].sysBuffer; }
  bool              isPerInstanceData(size_t buffer=0) const { return _buffers[buffer].perInstanceData; }
  void*             map( bool readonly, size_t buffer=0 );
  void              unmap();

private:
  struct Buffer
  {
    Buffer()
      : nVertices(0)
      , sysBuffer(NULL)
      , perInstanceData(false)
      {}

    sasVertexFormat format;
    size_t nVertices;
    void* sysBuffer;
    bool perInstanceData;
  } _buffers[kMaxBuffers];

  sasDeviceObject*  _deviceObject;
};


