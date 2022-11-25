#pragma once

class sasDeviceObject;
class sasDevice;

class sasConstantBuffer
{
  sasNO_COPY( sasConstantBuffer );
  sasMEM_OP( sasConstantBuffer );

public:
  sasConstantBuffer( size_t size );
  ~sasConstantBuffer();

  sasDeviceObject*  deviceObject();
  size_t            size() const { return _size; }
  void*             buffer() const { return _sysBuffer; }
  bool              isDirty() const { return _dirty; }
  void              validate() { _dirty = 0; }
  void*             map();
  void              unmap();  

private:
  sasDeviceObject*  _deviceObject;
  size_t            _size;
  void*             _sysBuffer;
  size_t            _dirty : 1;
};