#pragma once 

#include "sas_device_object.h"

class sasDeviceBuffer : public sasDeviceObject
{
  sasNO_COPY( sasDeviceBuffer );
  sasMEM_OP( sasDeviceBuffer );

public:
  sasDeviceBuffer( ID3D11Buffer* buffer, ID3D11UnorderedAccessView* uav, ID3D11ShaderResourceView* srv );
  ~sasDeviceBuffer();

  ID3D11Buffer* d3dBuffer() const { return _buffer; }    
  ID3D11ShaderResourceView* d3dSRV() const { return _srv; }
  ID3D11UnorderedAccessView* d3dUAV() const { return _uav; }

private:
  ID3D11Buffer* _buffer;
  ID3D11UnorderedAccessView* _uav;
  ID3D11ShaderResourceView* _srv;
};
