#include "sas_pch.h"
#include "sas_splashscreen.h"
#include "sas_resourcemng.h"
#include "sas_resource.h"
#include "sas_texture_resource.h"
#include "render/sas_texture.h"
#include "render/sas_rendertarget.h"
#include "sas_device.h"
#include "render/sas_miscresources.h"
#include "render/shaders/sas_shaderids.h"
#include "render/sas_constantbuffer.h"
#include "render/sas_rendermng.h"

sasSplashScreen* sasSplashScreen::sActiveSplashScreen = NULL;
std::vector< sasSplashScreen* > sasSplashScreen::sActiveWidgets;

struct sasQuadConstants
{
  smVec4 pos[ 4 ];
};

sasSplashScreen::sasSplashScreen( sasStringId splashScreenId, const char* texturePath, const smVec2& scale, const smVec2& position )
  : _id( splashScreenId )
  , _position( position )
  , _scale( scale )
{
  _texture = static_cast< sasTextureResource* >( sasResourceMng::singleton().load( texturePath ) );
  sasASSERT( _texture != NULL );

  _constants = sasNew sasConstantBuffer( sizeof( sasQuadConstants ) );
}

sasSplashScreen::~sasSplashScreen()
{
  sasDelete _constants;
  sasDelete _texture;
}

void sasSplashScreen::render( sasRenderTarget* rt )
{
  if( sasRenderMng::singletonPtr()->uiDisabled() )
    return;

  smVec4 rtSize = smVec4( static_cast< float >( rt->width() ), static_cast< float >( rt->height() ), 1.f, 1.f );
  smVec4 textureSize = smVec4( static_cast< float >( _texture->texture()->width() ), static_cast< float >(  _texture->texture()->height() ), 0.f, 1.f );
  smVec4 textureSizeRatio = textureSize / rtSize;
  smVec4 pos4 = smVec4( _position.X, _position.Y, 0.f, 1.f );
  smVec4 halfSize = ( textureSizeRatio ) * smVec4( _scale.X, _scale.Y, 1.f, 1.f );
  smVec4 tl = pos4 - halfSize;
  smVec4 br = pos4 + halfSize;
  sasQuadConstants* data = static_cast< sasQuadConstants* >( _constants->map() );
  if( data )
  {
    data->pos[ 0 ] = smVec4( tl.X, br.Y, 0.f, 1.f );
    data->pos[ 1 ] = smVec4( tl.X, tl.Y, 0.f, 1.f );
    data->pos[ 2 ] = smVec4( br.X, br.Y, 0.f, 1.f );
    data->pos[ 3 ] = smVec4( br.X, tl.Y, 0.f, 1.f );
  }
  _constants->unmap();

  sasDevice* device = sasDevice::singletonPtr();
  INSERT_PIX_EVENT("PIXEvent: render splash screen", 0xffffffff);
  device->setVertexShader( screenCopyShaderID | quadOverrideMask );
  device->setPixelShader( screenCopyShaderID | quadOverrideMask );
  device->setVertexBuffer( sasMiscResources::singletonPtr()->getFullScreenQuadVB() );
  device->setTexture( 0, _texture->texture(), sasDeviceUnit::PixelShader );
  device->setConstantBuffer( 1, _constants, sasDeviceUnit::VertexShader );
  device->setSamplerState( 0, sasSamplerState_Point_Clamp, sasDeviceUnit::PixelShader );
  device->setRenderTarget( 0, rt );
  device->setViewport( rt );
  device->setDepthStencilTarget( NULL, false );
  device->draw( sasPrimTopology_TriStrip, 4 );
  device->setTexture( 0, static_cast< sasTexture* >( NULL ), sasDeviceUnit::PixelShader );
  device->flushStates();
}

void sasSplashScreen::activateWidget( sasSplashScreen* widget )
{
  for( size_t i = 0; i < sActiveWidgets.size(); i++ )
  {
    if( sActiveWidgets[ i ] == widget )
    {
      //already active, do nothing
      return;
    }
  }
  sActiveWidgets.push_back( widget );
}

void sasSplashScreen::deactivateWidget( sasSplashScreen* widget )
{
  for( size_t i = 0; i < sActiveWidgets.size(); i++ )
  {
    if( sActiveWidgets[ i ] == widget )
    {
      sActiveWidgets[ i ] = sActiveWidgets[ sActiveWidgets.size() - 1 ];
      sActiveWidgets.pop_back();
      return;
    }
  }
}

void sasSplashScreen::renderWidgets( sasRenderTarget* targetBuffer )
{
  for( size_t i = 0; i < sActiveWidgets.size(); i++ )
  {
    sActiveWidgets[ i ]->render( targetBuffer );
  }
}
