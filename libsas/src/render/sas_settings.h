#pragma once

#include "utils/sas_singleton.h"

class sasDevice;

class sasTessellationSmoothing
{
public:
  enum Enum 
  {
    None,
    Phong,
    PNTriangles,
  };
};

struct sasResolution
{
public:
  sasResolution( uint32_t w, uint32_t h ) : _w( w ), _h( h ), _wf( static_cast< float >( w ) ), _hf( static_cast< float >( h ) ) {}
  sasResolution( float w, float h ) : _wf( w ), _hf( h ), _w( static_cast< uint32_t >( w ) ), _h( static_cast< uint32_t >( h ) ) {}

  uint32_t  width() const         { return _w; }
  uint32_t  height() const        { return _h; }
  float     widthFloat() const    { return _wf; }
  float     heightFloat() const   { return _hf; }

  void set( uint32_t w, uint32_t h ) { _w = w; _h = h; _wf = static_cast< float >( w ); _hf = static_cast< float >( h ); }
  void set( float w, float h ) { _wf = w; _hf = h; _w = static_cast< uint32_t >( w ); _h = static_cast< uint32_t >( h ); }

private:
  uint32_t  _w;
  uint32_t  _h;
  float     _wf;
  float     _hf;
};

class sasSettings : public sasSingleton<sasSettings>
{
  sasNO_COPY( sasSettings );
  sasMEM_OP( sasSettings );

public: //! ctor/dtor

  sasSettings(sasDevice* device);
  ~sasSettings(){}

public: //! methods

  sasResolution                   _frameBufferResolution;
  float                           _motionBlurIntensity;
  sasTessellationSmoothing::Enum  _tessellationMode;
  bool                            _fxaaEnabled;
  bool                            _ssaoEnabled;
  bool                            _volumetricLightsEnabled;
  bool                            _tessellationEnabled;
  bool                            _motionBlurEnabled;
  bool                            _shadowsEnabled;
  bool                            _godraysEnabled;
  bool                            _volumetricLightingEnabled;
  bool                            _heightFogEnabled;
  bool                            _particlesEnabled;
};
