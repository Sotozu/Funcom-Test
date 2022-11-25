#pragma once

class sasDeviceObject;
class sasDevice;

class sasIndirectArgsBuffer
{
  sasNO_COPY( sasIndirectArgsBuffer );
  sasMEM_OP( sasIndirectArgsBuffer );

public:
  sasIndirectArgsBuffer( size_t size, unsigned int* initialData, bool staging = false );
  ~sasIndirectArgsBuffer();

  sasDeviceObject*  deviceObject();
  size_t            size() const { return _size; }
  void*             buffer() const { return _sysBuffer; }
  bool              isDirty() const { return _dirty; }
  void              validate() { _dirty = 0; }
  bool              staging() const { return _staging; }

private:
  sasDeviceObject*  _deviceObject;
  size_t            _size;
  void*             _sysBuffer;
  size_t            _dirty : 1;
  size_t            _staging : 1;
};