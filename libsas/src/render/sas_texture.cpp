#include "sas_pch.h"
#include "sas_texture.h"
#include "sas_device_factory.h"

sasTexture::sasTexture( const sasTextureMipData* mipDatas, size_t nMips, size_t arraySize )
  : _deviceObject( 0 )
  , _mipDatas( 0 )
  , _nMips( 0 )
  , _arraySize( arraySize )
{
  _nMips = nMips;
  _mipDatas = (sasTextureMipData*)sasMalloc( _nMips * _arraySize * sizeof( sasTextureMipData ) );
  for( size_t i = 0; i < ( _nMips * arraySize ); i++ )
  {
    _mipDatas[ i ] = mipDatas[ i ];
  }
}

sasTexture::~sasTexture()
{
  if( _deviceObject && sasDeviceFactory::isValid() )
    sasDeviceFactory::singleton().releaseTexture( _deviceObject );  
  for( size_t i = 0; i < ( _nMips * _arraySize ); i++ )
  {
    sasFree( _mipDatas[ i ].data );
  }
  sasFree( _mipDatas );
}

sasDeviceObject*  sasTexture::deviceObject()
{
  if( !_deviceObject )
  {
    if( sasDeviceFactory::isValid() )
      _deviceObject = sasDeviceFactory::singleton().createTexture( *this );  
  }
  return _deviceObject;
}

void* sasTexture::buffer( size_t arraySlice, size_t mip ) const
{ 
  sasASSERT( mip < _nMips ); 
  sasASSERT( arraySlice < _arraySize );
  size_t offset = arraySlice * _nMips;
  return _mipDatas[ offset + mip ].data; 
}