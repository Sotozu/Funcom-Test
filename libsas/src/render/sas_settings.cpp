#include "sas_pch.h"
#include "sas_settings.h"
#include "sas_device.h"

sasSettings::sasSettings( sasDevice* device )
  : _ssaoEnabled( true )
  , _volumetricLightsEnabled( false )
  , _tessellationEnabled( false )
  , _motionBlurEnabled( true )
  , _tessellationMode( sasTessellationSmoothing::None )
  , _shadowsEnabled( true )
  , _godraysEnabled( true )
  , _fxaaEnabled( true )
  , _volumetricLightingEnabled( true )
  , _particlesEnabled( true )
  , _heightFogEnabled( false )
  , _frameBufferResolution( device->backBufferWidth(), device->backBufferHeight() )
{
  _tessellationEnabled = _tessellationEnabled && device->queryCapability( sasCapability_Tesselation );
  _volumetricLightsEnabled = _volumetricLightsEnabled && device->queryCapability( sasCapability_VolumetricLights );
  _motionBlurEnabled = _motionBlurEnabled && device->queryCapability( sasCapability_MotionBlur );
}

