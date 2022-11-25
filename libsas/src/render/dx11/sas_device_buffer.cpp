#include "sas_pch.h"
#include "sas_device_buffer.h"
#include "sas_device_common.h"

sasDeviceBuffer::sasDeviceBuffer( ID3D11Buffer* buffer, ID3D11UnorderedAccessView* uav, ID3D11ShaderResourceView* srv )
  : sasDeviceObject( sasDeviceObjectType::Buffer ),
    _buffer( buffer ),
    _srv( srv ),
    _uav( uav )
{
}

sasDeviceBuffer::~sasDeviceBuffer()
{
  DX_RELEASE( _buffer );
}

