#pragma once

#include "utils/sas_singleton.h"

#include "editor/sas_editor.h"

class sasRenderTarget;
class sasCamera;
class sasDevice;
class sasVertexBuffer;
class sasFrustumPoints;
class sasDepthStencilTarget;

class sasDebugMng : public sasSingleton< sasDebugMng >
{
  sasNO_COPY( sasDebugMng );  
  sasMEM_OP( sasDebugMng );

public:
  sasDebugMng();
  ~sasDebugMng();

  void applyDebugOptions( sasRenderTarget* diffuseBuffer, sasRenderTarget* normalBuffer, sasRenderTarget* lightBuffer, sasRenderTarget* specBuffer, sasRenderTarget* ssaoBuffer, 
                          sasRenderTarget* velocityBuffer, sasRenderTarget* volLightBuffer, sasRenderTarget* destBuffer );

  bool showBoundingVolumes() const { return _showBoundingVolumes; }
  bool showWireframe() const { return _showWireframe; }
  bool tessellationDisabled() const { return _disableTessellation; }
  smVec4  ssaoSettings() const { return smVec4( _ssaoScale, _ssaoBias, _ssaoIntensity, _ssaoDampen ); }
  bool showUVs() const { return _showUVs; }
  bool mouseInverted() const { return _mouseInverted; }
  bool controllerInverted() const { return _controllerInverted; }

  bool editMode() const { return _editMode; }
  void setEditMode( bool state ) { _editMode = state; }

  void setStereoEnabled( bool state )               { _vrStereoEnabled = state; }
  void setDistortionCompPassEnabled( bool state )   { _vrDistortionCompensation = state; }
  void setVSyncEnabled( bool state )                { _vSyncEnabled = state; }
  void setForceGPUFlush( bool state )               { _forceGpuFlush = state; }
  void setMotionBlurEnabled( bool state )           { _disableMotionBlur = !state; }
  void setVolumetricLightingEnabled( bool state )   { _enableVolumetricLighting = state; }
  bool view2dFluidBuffer() const                    { return _view2dFluidBuffer; }
  bool view3dFluidBuffer() const                    { return _view3dFluidBuffer; }

  void renderDebugObjects( sasCamera* camera, sasRenderTarget* rt, sasDepthStencilTarget* depthBuffer );
  void renderLights();
  void renderParticles();
  void renderFrustum( sasDevice& device, const sasFrustumPoints& frustum );
  bool takeFrustumShot() const      { return _takeFrustumShot; }
  void frustumShotTaken()           { _takeFrustumShot = false; }
  void captureFrustum()             { _takeFrustumShot = true; }
  bool showDebugFrustum() const     { return _showDebugFrustum; }
  bool lightCullingEnabled() const  { return _lightCullingEnabled; }
  void serializeInputSettings();
  bool loadInputSettingsFromDisk();
  bool newStyleRenderLists() const  { return _newStyleRenderLists; }

  void createOceanPreset();
  void deleteOceanPreset();
  void saveOceanPresets();
  void discardReloadOceanPresets();
  void editOceanPreset();
  void makeCurrentOceanPreset();

public:
  void setMainMenuVisibility( bool state );
  void setShadowMenuVisibility( bool state );
  void setSunLightMenuVisibility( bool state );
  void setDebugMiscMenuVisibility( bool state );
  void setMotionBlurMenuVisibility( bool state );
  void setOceanMenuVisibility( bool state );
  void setSsaoMenuVisibility( bool state );
  void setDebugViewMenuVisibility( bool state );
  void setEditorMenuVisibility( bool state );
  void setInputMenuVisibility( bool state );
  void setVRMenuVisibility( bool state );
  void setOceanPresetMenuVisibility( bool state );


private:
  // main debug tweak bar
  TwBar*  _twBar;
  bool    _enableGpuTimings;
  bool    _enableCpuTimings;

  bool   _editMode;

  // debug view tweak bar
  TwBar*  _twDebugViewBar;
  bool _showLightBuffer;
  bool _showSpecBuffer;
  bool _showNormalBuffer;
  bool _showDiffuseBuffer;
  bool _showSSAOBuffer;
  bool _showWireframe;
  bool _showBoundingVolumes;
  bool _showVelocityBuffer;
  bool _showGodrayBuffer;
  bool _showUVs;

  // ssao tweak bar
  TwBar*  _twSSAOBar;
  bool _disableSSAO;
  float _ssaoScale;
  float _ssaoIntensity;
  float _ssaoBias;
  float _ssaoDampen;

  // motion blur tweak bar
  TwBar*  _twMotionBlurBar;
  bool _disableMotionBlur;
  float _motionBlurIntensity;

  // ocean tweak bar
  TwBar*    _twOceanBar;
  bool      _oceanEnabled;
  bool      _oceanPresetEditMode;
  int       _oceanPresetIndex;
  int       _oceanLastFrameIndex;
  std::string   _oceanNewPresetName;

  TwBar*    _twOceanPresetBar;
  std::string   _oceanPresetName;
  int       _oceanNumLayers;
  int       _oceanLastFrameCurrentLayer;
  int       _oceanCurrentLayer;
  float     _oceanSpeed;
  float     _oceanWaveLength;
  float     _oceanAmplitude;
  float     _oceanSteepness;
  float     _oceanDirection[ 4 ];


  // debug options tweak bar
  TwBar*  _twDebugMiscBar;
  bool _enableCSLightPass;
  bool _currentCSLightPassState;
  bool _lightCullingEnabled;
  bool _enableVolumetricLighting;
  int  _volumetricLightDebugSlice;
  bool _view2dFluidBuffer;
  bool _view3dFluidBuffer;
  bool _heightFogEnabled;
  bool _particlesEnabled;
  bool _disableFXAA;
  bool _disableTessellation;
  uint8_t _tessellationMode;
  bool _takeFrustumShot;
  bool _showDebugFrustum;
  bool _newStyleRenderLists;
  bool _vSyncEnabled;
  bool _forceGpuFlush;

  // shadow tweak bar
  TwBar*  _twShadowBar;
  uint8_t _showShadowMap;
  bool  _shadowsEnabled;
  float _cascade0Range;
  float _cascade1Range;
  float _cascade2Range;
  float _normalOffset;
  bool _godraysEnabled;
  float _godraysIntensity;

  // directional tweak bar
  TwBar* _twSunlightBar;
  float _sunColour[ 3 ];
  float _sunIntensity;
  float _sunDir[ 3 ];

  // input tweak bar
  TwBar*  _twInputBar;
  bool    _controllerInverted;
  bool    _mouseInverted;

  // vr tweak bar
  TwBar*  _twVRBar;
  bool    _vrStereoEnabled;
  bool    _vrDistortionCompensation;
  float   _vrEyeSeparation;
  float   _vrLensSeparation;
  bool    _vrOverrideFov;
  bool    _vrFovVertical;


  // data for debug frustum rendering
  sasVertexBuffer* _frustumVB;

  static const int kInputDataVersion = 1;
};



