#pragma once

class sasDeviceObject;
class sasDevice;

class sasStructuredBuffer
{
  sasNO_COPY( sasStructuredBuffer );
  sasMEM_OP( sasStructuredBuffer );

public:
  sasStructuredBuffer( size_t size, size_t structuredByteStride, bool bDynamic, bool bAppendConsume, bool staging = false );
  ~sasStructuredBuffer();

  sasDeviceObject*  deviceObject();
  size_t            size() const { return _size; }
  void*             buffer() const { return _sysBuffer; }
  bool              isDirty() const { return _dirty; }
  void              validate() { _dirty = 0; }
  void*             map();
  void              unmap();  
  bool              dynamic() const { return _dynamic; }
  size_t            structuredByteStride() const { return _structuredByteStride; }
  bool              appendConsume() const { return _appendConsume; }
  bool              staging() const { return _staging; }

private:
  sasDeviceObject*  _deviceObject;
  size_t            _size;
  size_t            _structuredByteStride;
  void*             _sysBuffer;
  size_t            _dirty : 1;
  size_t            _dynamic : 1;
  size_t            _appendConsume : 1;
  size_t            _staging : 1;
};