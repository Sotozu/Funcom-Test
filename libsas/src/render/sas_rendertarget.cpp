#include "sas_pch.h"
#include "sas_rendertarget.h"
#include "sas_device_factory.h"
#include "sas_device_rendertarget.h"
#include "sas_device_texture.h"

sasRenderTarget::sasRenderTarget( const sasRenderTargetDesc& rtDesc )
  : _deviceObject(0)
  , _desc( rtDesc )
{
  _widthFloat = static_cast< float >( rtDesc.width );
  _heightFloat = static_cast< float >( rtDesc.height );
}

sasRenderTarget::~sasRenderTarget()
{
  if( _deviceObject && sasDeviceFactory::isValid() )
    sasDeviceFactory::singleton().releaseRenderTarget(_deviceObject);
}

sasDeviceObject*  sasRenderTarget::deviceObject()
{
  if( !_deviceObject )
  {
    if( sasDeviceFactory::isValid() )
      _deviceObject = sasDeviceFactory::singleton().createRenderTarget( *this );  
  }
  return _deviceObject;
}

sasRenderTargetSetDeviceObject::sasRenderTargetSetDeviceObject( sasRenderTarget* rt, sasDeviceObject* dob )
{
  sasASSERT( rt && dob && dob->isA(sasDeviceObjectType::RenderTarget) );

  sasDeviceRenderTarget* drt = (sasDeviceRenderTarget*)dob;

  rt->_deviceObject = drt;
  rt->_desc.width   = drt->width();
  rt->_desc.height  = drt->height();  
}


sasPingPongRT::sasPingPongRT( sasRenderTarget* ping, sasRenderTarget* pong )
  : _currentBufferIndex( 0 )
{
  _rts[ 0 ] = ping;
  _rts[ 1 ] = pong;
}