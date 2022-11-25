#pragma once 

#include "utils/sas_singleton.h"
#include "sas_rendertypes.h"

struct sasConfig;
class sasShaderMng;
class sasDevice;
class sasConstantBuffer;
class sasVisibility;
class sasRenderStats;
class sasRenderTargetPool;
class sasMiscResources;
class sasGui;
class sasRenderTarget;
class sasLightPass;
class sasDrawList;
class sasDebugMng;
class sasDebugRender;
class sasSSAO;
class sasVolumetricLightMng;
class sasSettings;
class sasDestructionMng;
class sasMotionBlur;
class sasShadowMng;
class sasDestructionParticles;
class sasSPHFluidSolver;
class sasProbeMng;
class sasCubeMapGenerator;
class sasDepthStencilTarget;
class sasPostAA;
class sasEditor;
class sasRenderList;
class sasGpuProfiler;
class sasCpuProfiler;
class sasStereo;
class sasGPUParticleMng;
class sas2dFluidSolver;
class sas3dFluidSolver;
class sasHeightFog;
class sasDoF;
class sasCompositePass;
class sasSSR;
class sasOceanVolume;
class sasSandSimMng;

class sasRenderMng : public sasSingleton< sasRenderMng >
{  
  sasNO_COPY( sasRenderMng );
  sasMEM_OP( sasRenderMng );

public:
  sasRenderMng( const sasConfig& config );
  ~sasRenderMng();

public:
  void        update();

  void        copyRenderTarget( sasRenderTarget* src, sasRenderTarget* dst, sasSamplerState samplerState = sasSamplerState_Point_Clamp );
  void        copyDepthTargetToRenderTarget( sasDepthStencilTarget* src, sasRenderTarget* dst );
  void        renderBoundingSphere( const smMat44& xform, const smVec4& colour );
  void        setCSLightPassState( bool enable );

  void        loadEngineResources();

  void        setUIEnabled( bool state )  { _uiDisabled = !state; } 
  bool        uiDisabled() const          { return _uiDisabled; }

private:
  void        shutdown();
  void        adjustBackBufferSize( );
  void        renderFrame( );
  void        updatePostFrame();

private:    
  sasShaderMng*             _shaderMng;
  sasDevice*                _sasDevice;
  sasConstantBuffer*        _constantBuffer; 
  sasVisibility*            _visibilityMng;
  sasDrawList*              _mainDrawList;
  sasRenderList*            _mainRenderList;
  sasRenderStats*           _stats;
  sasRenderTargetPool*      _rtPool;
  sasMiscResources*         _miscResources;
  sasLightPass*             _lightPass;
  sasGui*	                  _gui;
  sasDebugMng*              _debugMng;
  sasDebugRender*           _debugRender;
  sasSettings*              _renderSettings;
  sasShadowMng*             _shadowMng;
  sasPostAA*                _postAA;
  sasEditor*                _editor;
  sasGpuProfiler*           _gpuProfiler;
  sasStereo*                _stereo;
  sasCompositePass*         _compositePass;

  bool                      _uiDisabled;
};