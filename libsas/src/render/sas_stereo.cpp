#include "sas_pch.h"
#include "sas_stereo.h"
#include "utils/sas_timer.h"
#include "sas_miscresources.h"
#include "sas_device.h"
#include "sas_constantbuffer.h"
#include "sas_rendertarget.h"
#include "sas_rendertargetpool.h"
#include "sas_rendermng.h"
#include "shaders/sas_shaderids.h"
#include "utils/sas_camera.h"
#include "utils/sas_cameramng.h"
#include "render/sas_settings.h"

#include "OVR.h"

#ifdef PLATFORM_DX11
#include "sas_device_rendertarget.h"
#include "sas_device_texture.h"
#endif

struct StereoCB
{
  smMat44 matProjViewStereo[ 2 ];
  smMat44 matViewStereo[ 2 ];
  smMat44 matProjStereo[ 2 ];
  smMat44 matInvViewStereo[ 2 ];
  smMat44 matInvProjStereo[ 2 ];
  smMat44 matInvProjViewStereo[ 2 ];
  smVec4  cameraPositionStereo[ 2 ];
};

struct VSDistortionCB
{
  smMat44 uvXform;
};

struct PSDistortionCB
{
  smVec4 lensCenter_ScreenCenter;
  smVec4 scale_ScaleIn;
  smVec4 warpParams;
};

sasStereo::sasStereo( const sasConfig& config )
  : _enabled( false )
  , _distortionCompPassEnabled( false )
  , _lensSeparationDistance( 0.f )
  , _eyeSeparationDistance( 0.65f )
  , _overrideFov( true )
  , _verticalFovSupplied( true )
  , _hmd( static_cast< ovrHmd >( config.vrHmd ) )
{
  for( unsigned int i = 0; i < sasStereoView::Count; i++ )
  {
    _viewports[ i ].minDepth = 0.f;
    _viewports[ i ].maxDepth = 1.f;
    _viewports[ i ].topLeftX = 0;
    _viewports[ i ].topLeftY = 0;
    _viewports[ i ].width = sasSettings::singletonPtr()->_frameBufferResolution.width();
    _viewports[ i ].height = sasSettings::singletonPtr()->_frameBufferResolution.height();
  }

  _stereoConstants = sasNew sasConstantBuffer( sizeof( StereoCB ) );
  _vsDistortionCB = sasNew sasConstantBuffer( sizeof( VSDistortionCB ) );
  _psDistortionCB = sasNew sasConstantBuffer( sizeof( PSDistortionCB ) );

  #ifdef PLATFORM_DX11
  if( _hmd != nullptr )
  {
    sasDevice::singletonPtr()->registerDeviceResetCallback( &sasStereo::InitializeOvrRenderingD3D11 );
    bool r = InitializeOvrRenderingD3D11();
    if( r )
      _enabled = true;

    sasASSERT( r );
  }
  #endif

}

sasStereo::~sasStereo()
{
  sasDelete _psDistortionCB;
  sasDelete _vsDistortionCB;
  sasDelete _stereoConstants;
}

#ifdef PLATFORM_DX11
bool sasStereo::InitializeOvrRenderingD3D11()
{
  sasDevice* d = sasDevice::singletonPtr();
  sasStereo* stereo = sasStereo::singletonPtr();
  ovrHmd_AttachToWindow( stereo->_hmd, d->_hwnd, nullptr, nullptr );

  ovrFovPort eyeFov[ 2 ] = { stereo->_hmd->DefaultEyeFov[ 0 ], stereo->_hmd->DefaultEyeFov[ 1 ] } ;

  ovrD3D11Config d3d11cfg;
  d3d11cfg.D3D11.Header.API = ovrRenderAPI_D3D11;
  d3d11cfg.D3D11.Header.Multisample = 0;
  d3d11cfg.D3D11.pDevice = d->d3dDevice();
  d3d11cfg.D3D11.pDeviceContext = d->d3dDeviceContext();
  d3d11cfg.D3D11.pBackBufferRT = static_cast< sasDeviceRenderTarget* >( d->backBuffer()->deviceObject() )->d3dRTV();
  d3d11cfg.D3D11.pSwapChain = d->_swapChain;
  if( !ovrHmd_ConfigureRendering( stereo->_hmd, &d3d11cfg.Config, ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive | ovrDistortionCap_NoRestore, 
                                    eyeFov, stereo->_eyeRenderDesc ) )
  {
    return false;
  }

  OVR::Sizei recommenedTex0Size = ovrHmd_GetFovTextureSize( stereo->_hmd, ovrEye_Left, stereo->_hmd->DefaultEyeFov[ 0 ], 1.f );
  OVR::Sizei recommenedTex1Size = ovrHmd_GetFovTextureSize( stereo->_hmd, ovrEye_Right, stereo->_hmd->DefaultEyeFov[ 1 ], 1.f );

  OVR::Sizei renderTargetSize;
  renderTargetSize.w = sasSettings::singletonPtr()->_frameBufferResolution.width();//recommenedTex0Size.w + recommenedTex1Size.w;
  renderTargetSize.h = sasSettings::singletonPtr()->_frameBufferResolution.height();//max( recommenedTex0Size.h, recommenedTex1Size.h );
  
  sasSettings::singletonPtr()->_frameBufferResolution.set( (uint32_t)renderTargetSize.w, (uint32_t)renderTargetSize.h );

  ovrRecti eyeRenderViewport[ 2 ];
  eyeRenderViewport[ 0 ].Pos  = OVR::Vector2i(0,0);
  eyeRenderViewport[ 0 ].Size = OVR::Sizei( sasSettings::singletonPtr()->_frameBufferResolution.width() / 2, sasSettings::singletonPtr()->_frameBufferResolution.height() );
  eyeRenderViewport[ 1 ].Pos  = OVR::Vector2i( ( sasSettings::singletonPtr()->_frameBufferResolution.width() + 1 ) / 2, 0);
  eyeRenderViewport[ 1 ].Size = eyeRenderViewport[ 0 ].Size;

  //actual texture ptrs will be updated at present time
  stereo->_eyeTexture[ 0 ].D3D11.Header.API            = ovrRenderAPI_D3D11;
  stereo->_eyeTexture[ 0 ].D3D11.Header.TextureSize    = renderTargetSize;
  stereo->_eyeTexture[ 0 ].D3D11.Header.RenderViewport = eyeRenderViewport[ 0 ];
  stereo->_eyeTexture[ 1 ] = stereo->_eyeTexture[0];
  stereo->_eyeTexture[ 1 ].D3D11.Header.RenderViewport = eyeRenderViewport[ 1 ];

  return true;
}
#endif

void sasStereo::stepPreRender( sasCamera& camera )
{
  if( !_enabled )
    return;

  size_t w = sasSettings::singletonPtr()->_frameBufferResolution.width();
  size_t h = sasSettings::singletonPtr()->_frameBufferResolution.height();

  float wFloat = static_cast< float >( w );
  float hFloat = static_cast< float >( h );

  camera.setFOV( 90.f );
  
  _eyeRenderPose[ ovrEye_Left ] = ovrHmd_GetHmdPosePerEye( _hmd, ovrEye_Left );
  _eyeRenderPose[ ovrEye_Right ] = ovrHmd_GetHmdPosePerEye( _hmd, ovrEye_Right );

  camera.getViewMatrix( _viewMtx[ sasStereoView::Mono ] );
  camera.getProjMatrix( wFloat / 2.f, hFloat, _projMtx[ sasStereoView::Mono ] );

  ovrMatrix4f ovrLeftProj = ovrMatrix4f_Projection( _eyeRenderDesc[ ovrEye_Left ].Fov, camera.zNear(), camera.zFar(), true );
  ovrMatrix4f ovrRightProj = ovrMatrix4f_Projection( _eyeRenderDesc[ ovrEye_Right ].Fov, camera.zNear(), camera.zFar(), true );
  _projMtx[ sasStereoView::LeftEye ].set( static_cast< float* >( &( ovrLeftProj.M[ 0 ][ 0 ] ) ) );
  _projMtx[ sasStereoView::RightEye ].set( static_cast< float* >( &( ovrRightProj.M[ 0 ][ 0 ] ) ) );

   camera.getViewMatrix( _viewMtx[ sasStereoView::LeftEye ] );
   camera.getViewMatrix( _viewMtx[ sasStereoView::RightEye ] );

  _viewports[ sasStereoView::Mono ].width = static_cast< uint16_t >( w );
  _viewports[ sasStereoView::Mono ].height = static_cast< uint16_t >( h );

  _viewports[ sasStereoView::LeftEye ].width = static_cast< uint16_t >( w ) / 2;
 
  _viewports[ sasStereoView::RightEye ].width = static_cast< uint16_t >( w ) / 2;
  _viewports[ sasStereoView::RightEye ].topLeftX = static_cast< uint16_t >( w ) / 2;

  smMat44 matProjViewStereo[ 2 ];
  smMat44 matViewStereo[ 2 ];
  smMat44 matProjStereo[ 2 ];
  smMat44 matInvViewStereo[ 2 ];
  smMat44 matInvProjStereo[ 2 ];
  smMat44 matInvProjViewStereo[ 2 ];
  smVec4  cameraPositionStereo[ 2 ];

  smVec4 camRight;
  camera.getRight( camRight );

  StereoCB* cbData = static_cast< StereoCB* >( _stereoConstants->map() );
  cbData->matViewStereo[ 0 ] = _viewMtx[ sasStereoView::LeftEye ];
  cbData->matViewStereo[ 1 ] = _viewMtx[ sasStereoView::RightEye ];
  cbData->matProjStereo[ 0 ] = _projMtx[ sasStereoView::LeftEye ];
  cbData->matProjStereo[ 1 ] = _projMtx[ sasStereoView::RightEye ];
  smMul( _projMtx[ sasStereoView::LeftEye ], _viewMtx[ sasStereoView::LeftEye ], cbData->matProjViewStereo[ 0 ] );
  smMul( _projMtx[ sasStereoView::RightEye ], _viewMtx[ sasStereoView::RightEye ], cbData->matProjViewStereo[ 1 ] );
  smInverse( _viewMtx[ sasStereoView::LeftEye ], cbData->matInvViewStereo[ 0 ] );
  smInverse( _viewMtx[ sasStereoView::RightEye ], cbData->matInvViewStereo[ 1 ] );
  smInverse( _projMtx[ sasStereoView::LeftEye ], cbData->matInvProjStereo[ 0 ] );
  smInverse( _projMtx[ sasStereoView::RightEye ], cbData->matInvProjStereo[ 1 ] );
  smInverse( cbData->matProjViewStereo[ 0 ], cbData->matInvProjViewStereo[ 0 ] );
  smInverse( cbData->matProjViewStereo[ 1 ], cbData->matInvProjViewStereo[ 1 ] );
  cbData->cameraPositionStereo[ 0 ] = camera.position() + camRight * _eyeRenderDesc[ ovrEye_Left ].HmdToEyeViewOffset.x;
  cbData->cameraPositionStereo[ 1 ] = camera.position() + camRight * _eyeRenderDesc[ ovrEye_Right ].HmdToEyeViewOffset.x;
  _stereoConstants->unmap();

  sasDevice::singletonPtr()->setConstantBuffer( 4, _stereoConstants, sasDeviceUnit::VertexShader );
  sasDevice::singletonPtr()->setConstantBuffer( 4, _stereoConstants, sasDeviceUnit::HullShader );
  sasDevice::singletonPtr()->setConstantBuffer( 4, _stereoConstants, sasDeviceUnit::DomainShader );
  sasDevice::singletonPtr()->setConstantBuffer( 4, _stereoConstants, sasDeviceUnit::GeometryShader );
  sasDevice::singletonPtr()->setConstantBuffer( 4, _stereoConstants, sasDeviceUnit::PixelShader );
  sasDevice::singletonPtr()->setConstantBuffer( 4, _stereoConstants, sasDeviceUnit::ComputeShader );
}

void sasStereo::updateViewData( sasStereoView::Enum view, bool firstCallForEye )
{
  sasMiscResources* miscRsc = sasMiscResources::singletonPtr();
  sasDevice* device = sasDevice::singletonPtr();

  miscRsc->updateGlobalShaderConstants( sasSettings::singletonPtr()->_frameBufferResolution.widthFloat(), sasSettings::singletonPtr()->_frameBufferResolution.heightFloat(), 
                                          &_viewMtx[ view ], &_projMtx[ view ], firstCallForEye, view );
  device->setViewport( _viewports[ view ] );
}

void sasStereo::vrBeginFrame()
{
  //ovrHmd_BeginFrame( _hmd, 0 );
}

void sasStereo::vrPresentFrame( sasRenderTarget* frameRT )
{
  _eyeTexture[ 0 ].D3D11.pTexture = _eyeTexture[ 1 ].D3D11.pTexture = static_cast< sasDeviceRenderTarget* >( frameRT->deviceObject() )->textureObj()->d3dTexture();
  _eyeTexture[ 0 ].D3D11.pSRView = _eyeTexture[ 1 ].D3D11.pSRView = static_cast< sasDeviceRenderTarget* >( frameRT->deviceObject() )->textureObj()->d3dShaderResourceView();
  ovrHmd_EndFrame( _hmd, _eyeRenderPose, &_eyeTexture[0].Texture );
}

void sasStereo::applyDistortionCorrection( sasRenderTarget* inputRT, sasRenderTarget* outputRT )
{
  return;

  if( !_enabled || !_distortionCompPassEnabled )
    return; 

  /*sasASSERT( inputRT && outputRT );
  sasDevice* device = sasDevice::singletonPtr();

  sasRenderTargetDesc rtDesc;
  rtDesc.width = outputRT->width();
  rtDesc.height = outputRT->height();
  rtDesc.format = sasPixelFormat::RGBA_U8;
  rtDesc.mips = 1;
  rtDesc.needUAV = false;
  rtDesc.type = sasTextureType::sasTexture2D;
  rtDesc.depth = 0;
  sasRenderTarget* tempBuffer = sasRenderTargetPool::singletonPtr()->RequestAndLock( rtDesc );

  sasViewport vp;

  //left eye
  vp.width = outputRT->width() / 2;
  vp.height = outputRT->height();

  float w = float( vp.width ) / float( outputRT->width() ),
    h = float( vp.height ) / float( outputRT->height() ),
    x = float( vp.topLeftX ) / float( outputRT->width() ),
    y = float( vp.topLeftY ) / std::max< float >( float( outputRT->height() ), 0.1f );
  float as = float( vp.width ) / std::max< float >( float( vp.height ), 0.1f );
  float scaleFactor = 1.f / _ovrStereoConfig.GetDistortionConfig().Scale;

  PSDistortionCB* cbData = static_cast< PSDistortionCB* >( _psDistortionCB->map() );
  cbData->lensCenter_ScreenCenter.X = x + ( w + _ovrStereoConfig.GetDistortionConfig().XCenterOffset * 0.5f ) * 0.5f;
  cbData->lensCenter_ScreenCenter.Y = y + h * 0.5f;
  cbData->lensCenter_ScreenCenter.Z = x + w * 0.5f;
  cbData->lensCenter_ScreenCenter.W = y + h * 0.5f;
  cbData->scale_ScaleIn.X = ( w / 2.f ) * scaleFactor;
  cbData->scale_ScaleIn.Y = ( h / 2.f ) * scaleFactor * as;
  cbData->scale_ScaleIn.Z = ( 2.f / w );
  cbData->scale_ScaleIn.W = ( 2.f / h ) / as;
  cbData->warpParams = smVec4( _ovrStereoConfig.GetDistortionConfig().K[0], _ovrStereoConfig.GetDistortionConfig().K[1], 
                                _ovrStereoConfig.GetDistortionConfig().K[2], _ovrStereoConfig.GetDistortionConfig().K[3] );
  _psDistortionCB->unmap();

  VSDistortionCB* cbDataVS = static_cast< VSDistortionCB* >( _vsDistortionCB->map() );
  cbDataVS->uvXform.setIdentity();
  cbDataVS->uvXform.L0 = smVec4( w, 0.f, 0.f, x );
  cbDataVS->uvXform.L1 = smVec4( 0.f, h, 0.f, y );
  _vsDistortionCB->unmap();

  device->setConstantBuffer( 1, _vsDistortionCB, sasDeviceUnit::VertexShader );
  device->setConstantBuffer( 1, _psDistortionCB, sasDeviceUnit::PixelShader );
  device->setVertexBuffer( sasMiscResources::singletonPtr()->getFullScreenQuadVB() );
  device->setViewport( vp );
  device->setVertexShader( ovrDistortionShaderID );
  device->setPixelShader( ovrDistortionShaderID );
  device->setTexture( 0, inputRT, sasDeviceUnit::PixelShader );
  device->setSamplerState( 0, sasSamplerState_Bilinear_Clamp, sasDeviceUnit::PixelShader );
  device->setRenderTarget( 0, tempBuffer );
  device->setDepthStencilTarget( NULL, false );
  device->draw( sasPrimTopology_TriStrip, 4 );

  //right eye
  vp.width = outputRT->width() / 2;
  vp.topLeftX = outputRT->width() / 2;
  vp.topLeftY = 0;
  vp.height = outputRT->height();

  w = float( vp.width ) / float( outputRT->width() );
  h = float( vp.height ) / float( outputRT->height() );
  x = float( vp.topLeftX ) / float( outputRT->width() );
  y = float( vp.topLeftY ) / std::max< float >( float( outputRT->height() ), 0.1f );

  cbData = static_cast< PSDistortionCB* >( _psDistortionCB->map() );
  cbData->lensCenter_ScreenCenter.X = x + ( w - _ovrStereoConfig.GetDistortionConfig().XCenterOffset * 0.5f ) * 0.5f;
  cbData->lensCenter_ScreenCenter.Y = y + h * 0.5f;
  cbData->lensCenter_ScreenCenter.Z = x + w * 0.5f;
  cbData->lensCenter_ScreenCenter.W = y + h * 0.5f;
  cbData->scale_ScaleIn.X = ( w / 2.f ) * scaleFactor;
  cbData->scale_ScaleIn.Y = ( h / 2.f ) * scaleFactor * as;
  cbData->scale_ScaleIn.Z = ( 2.f / w );
  cbData->scale_ScaleIn.W = ( 2.f / h ) / as;
  cbData->warpParams = smVec4( _ovrStereoConfig.GetDistortionConfig().K[0], _ovrStereoConfig.GetDistortionConfig().K[1], 
    _ovrStereoConfig.GetDistortionConfig().K[2], _ovrStereoConfig.GetDistortionConfig().K[3] );
  _psDistortionCB->unmap();

  cbDataVS = static_cast< VSDistortionCB* >( _vsDistortionCB->map() );
  cbDataVS->uvXform.setIdentity();
  cbDataVS->uvXform.L0 = smVec4( w, 0.f, 0.f, x );
  cbDataVS->uvXform.L1 = smVec4( 0.f, h, 0.f, y );
  _vsDistortionCB->unmap();

  device->setConstantBuffer( 1, _vsDistortionCB, sasDeviceUnit::VertexShader );
  device->setConstantBuffer( 1, _psDistortionCB, sasDeviceUnit::PixelShader );
  device->setViewport( vp );
  device->draw( sasPrimTopology_TriStrip, 4 );

  device->setRenderTarget( 0, static_cast< sasRenderTarget* >( NULL ) );
  device->setTexture( 0, static_cast< sasTexture* >( NULL ), sasDeviceUnit::PixelShader );
  device->flushStates();

  //copy to dest rt
  sasRenderMng::singletonPtr()->copyRenderTarget( tempBuffer, outputRT );
  sasRenderTargetPool::singletonPtr()->Unlock( tempBuffer );*/
}

//void sasStereo::setOvrSteroConfig( const OVR::Util::Render::StereoConfig& ovrStereoConfig )
//{
//  _ovrStereoConfig = ovrStereoConfig;
//  _eyeSeparationDistance = _ovrStereoConfig.GetProjectionCenterOffset();
//}
