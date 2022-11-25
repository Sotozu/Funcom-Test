#pragma once 

#include "sas_rendertypes.h"

class sasDeviceObject;

struct sasTextureMipData
{
  size_t          width;
  size_t          height;
  sasPixelFormat::Enum  format;
  unsigned char*  data;
  size_t          dataSize;  
  size_t          pitch;

  sasTextureMipData()
  {
    width = height = 0;
    format = sasPixelFormat::NONE;
    data = 0;
    dataSize = 0;
    pitch = 0;
  }
};

class sasTexture
{
  sasNO_COPY( sasTexture );
  sasMEM_OP( sasTexture );

public:
  sasTexture( const sasTextureMipData* mipData, size_t nMips, size_t arraySize );
  ~sasTexture();

public:
  sasDeviceObject*    deviceObject();  
  size_t              width( size_t mip = 0 ) const    { sasASSERT( mip<_nMips ); return _mipDatas[ mip ].width; }
  size_t              height( size_t mip = 0 ) const   { sasASSERT( mip<_nMips ); return _mipDatas[ mip ].height; }
  size_t              mipCount() const                 { return _nMips; }
  size_t              arraySize() const                { return _arraySize; }
  sasPixelFormat::Enum format( size_t mip = 0 ) const  { sasASSERT( mip<_nMips ); return _mipDatas[ mip ].format; }
  size_t              pitch( size_t mip = 0 ) const    { sasASSERT( mip<_nMips ); return _mipDatas[ mip ].pitch; }
  size_t              dataSize( size_t mip = 0 ) const { sasASSERT( mip<_nMips ); return _mipDatas[ mip ].dataSize; }
  void*               buffer( size_t arraySlice, size_t mip ) const;

private:
  sasDeviceObject*    _deviceObject;
  sasTextureMipData*  _mipDatas;
  size_t              _nMips;  
  size_t              _arraySize;
};


