#pragma once

#include "utils/sas_singleton.h"
#include "sas_rendertypes.h"

#include "OVR.h"

#ifdef PLATFORM_DX11
#define OVR_D3D_VERSION 11
#include "../Src/OVR_CAPI_D3D.h"
#endif

class sasConstantBuffer;
class sasRenderTarget;


namespace sasStereoView
{
  enum Enum
  {
    Mono = 0,
    LeftEye,
    RightEye,
    
    Count,
  };
}

class sasStereo : public sasSingleton< sasStereo >
{
  sasNO_COPY( sasStereo );
  sasMEM_OP( sasStereo );

public: 

  sasStereo( const sasConfig& config );
  ~sasStereo();

  bool enabled() const                            { return ( _hmd != nullptr ); }
 
  float eyeSeparationDistance() const             { return _eyeSeparationDistance; }
  void setEnabled( bool state )                   { /*_enabled = state;*/ }
  void setDistortionCompPassEnabled( bool state ) { _distortionCompPassEnabled = state; }
  void setOverrideFov( bool state )               { _overrideFov = state; }
  void setVerticalFov( bool state )               { _verticalFovSupplied = state; }
  void setEyeSeparation( float sep )              { _eyeSeparationDistance = sep; }
  void setLensSeparation( float sep )             { _lensSeparationDistance = sep; }

  void stepPreRender( sasCamera& camera );
  void updateViewData( sasStereoView::Enum view, bool firstCallForEye = true );

  void applyDistortionCorrection( sasRenderTarget* inputRT, sasRenderTarget* outputRT );

  void vrBeginFrame();
  void vrPresentFrame( sasRenderTarget* frameRT );

  //void setOvrSteroConfig( const OVR::Util::Render::StereoConfig& ovrStereoConfig );

private:
  static bool InitializeOvrRenderingD3D11();

private: 
  ovrHmd              _hmd;
  ovrEyeRenderDesc    _eyeRenderDesc[ 2 ];
  ovrPosef            _eyeRenderPose[ 2 ];
#ifdef PLATFORM_DX11
  ovrD3D11Texture    _eyeTexture[ 2 ];
#endif
  
  sasConstantBuffer* _stereoConstants;

  smMat44 _viewMtx[ sasStereoView::Count ];
  smMat44 _projMtx[ sasStereoView::Count ];
  smMat44 _projViewMtx[ sasStereoView::Count ];
  smMat44 _invProjViewMtx[ sasStereoView::Count ];
  sasViewport _viewports[ sasStereoView::Count ];
  
  sasConstantBuffer* _vsDistortionCB;
  sasConstantBuffer* _psDistortionCB;

  float _lensSeparationDistance;
  float _eyeSeparationDistance;
  bool  _distortionCompPassEnabled;
  bool  _enabled;

  bool _overrideFov;
  bool _verticalFovSupplied;

};
