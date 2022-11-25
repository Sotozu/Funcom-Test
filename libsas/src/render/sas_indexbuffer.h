#pragma once

class sasDeviceObject;

class sasIndexBuffer
{
  sasNO_COPY( sasIndexBuffer );
  sasMEM_OP( sasIndexBuffer );

public:
  sasIndexBuffer( size_t nIndices );
  ~sasIndexBuffer();

public:
  sasDeviceObject* deviceObject();
  size_t size() const         { return _nIndices*2; }
  size_t indicesCount() const { return _nIndices; }
  void* buffer() const        { return _sysBuffer; }

  unsigned short* map();
  void unmap();

private:  
  sasDeviceObject*  _deviceObject;
  size_t            _nIndices;
  void*             _sysBuffer;
};
