#include "sas_pch.h"
#include "sas_debugmng.h"
#include "render/sas_rendermng.h"
#include "sas_device.h"
#include "render/sas_rendertarget.h"
#include "render/shaders/sas_shadermng.h"
#include "render/sas_settings.h"
#include "sas_device.h"
#include "render/sas_constantbuffer.h"
#include "utils/sas_camera.h"
#include "render/sas_miscresources.h"
#include "render/shaders/sas_shaderids.h"
#include "render/sas_vertexbuffer.h"
#include "render/shadows/sas_shadowmng.h"
#include "utils/sas_frustum.h"
#include "render/lights/sas_lightmng.h"
#include "editor/sas_editor.h"
#include "sas_debugrender.h"
#include "render/lights/sas_light.h"
#include "utils/sas_cameramng.h"
#include "resource/sas_resourcemng.h"
#include "utils/sas_file.h"
#include "render/profiling/sas_gpuprofiler.h"
#include "render/profiling/sas_cpuprofiler.h"
#include "render/sas_stereo.h"

void TW_CALL reloadShaders(void*)
{
  sasShaderMng::singletonPtr()->releaseShaders();
  sasDebugOutput("Reloaded all shaders\n");
}

void TW_CALL captureFrustumTw(void*)
{
  sasDebugMng::singletonPtr()->captureFrustum();
  sasDebugOutput("Captured frustum\n");
}

void TW_CALL mainToShadowMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( false );
  sasDebugMng::singletonPtr()->setShadowMenuVisibility( true );
  sasDebugOutput("To shadow menu\n");
}

void TW_CALL shadowToMainMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( true );
  sasDebugMng::singletonPtr()->setShadowMenuVisibility( false );
  sasDebugOutput("To main menu\n");
}

void TW_CALL mainToSunLightMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( false );
  sasDebugMng::singletonPtr()->setSunLightMenuVisibility( true );
  sasDebugOutput("To shadow menu\n");
}

void TW_CALL sunLightToMainMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( true );
  sasDebugMng::singletonPtr()->setSunLightMenuVisibility( false );
  sasDebugOutput("To main menu\n");
}

void TW_CALL mainToBufferViewMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( false );
  sasDebugMng::singletonPtr()->setDebugViewMenuVisibility( true );
  sasDebugOutput("To buffer view menu\n");
}

void TW_CALL viewBuffersToMainMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( true );
  sasDebugMng::singletonPtr()->setDebugViewMenuVisibility( false );
  sasDebugOutput("To main menu\n");
}

void TW_CALL mainToSsaoMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( false );
  sasDebugMng::singletonPtr()->setSsaoMenuVisibility( true );
  sasDebugOutput("To ssao menu\n");
}

void TW_CALL ssaoToMainMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( true );
  sasDebugMng::singletonPtr()->setSsaoMenuVisibility( false );
  sasDebugOutput("To main menu\n");
}

void TW_CALL mainToMotionBlurMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( false );
  sasDebugMng::singletonPtr()->setMotionBlurMenuVisibility( true );
  sasDebugOutput("To motion blur menu\n");
}

void TW_CALL motionBlurToMainMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( true );
  sasDebugMng::singletonPtr()->setMotionBlurMenuVisibility( false );
  sasDebugOutput("To main menu\n");
}

void TW_CALL mainToOceanMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( false );
  sasDebugMng::singletonPtr()->setOceanMenuVisibility( true );
  sasDebugOutput("To ocean menu\n");
}

void TW_CALL oceanToMainMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( true );
  sasDebugMng::singletonPtr()->setOceanMenuVisibility( false );
  sasDebugOutput("To main menu\n");
}

void TW_CALL mainToMiscSettingsMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( false );
  sasDebugMng::singletonPtr()->setDebugMiscMenuVisibility( true );
  sasDebugOutput("To misc settings menu\n");
}

void TW_CALL miscSettingsToMainMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( true );
  sasDebugMng::singletonPtr()->setDebugMiscMenuVisibility( false );
  sasDebugOutput("To main menu\n");
}

void TW_CALL mainToInputMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( false );
  sasDebugMng::singletonPtr()->setInputMenuVisibility( true );
  sasDebugOutput("To input menu\n");
}

void TW_CALL inputToMainMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( true );
  sasDebugMng::singletonPtr()->setInputMenuVisibility( false );
  sasDebugOutput("To main menu\n");
}

void TW_CALL mainToVRMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( false );
  sasDebugMng::singletonPtr()->setVRMenuVisibility( true );
  sasDebugOutput("To VR menu\n");
}

void TW_CALL vrToMainMenu(void*)
{
  sasDebugMng::singletonPtr()->setMainMenuVisibility( true );
  sasDebugMng::singletonPtr()->setVRMenuVisibility( false );
  sasDebugOutput("To main menu\n");
}


void TW_CALL saveInputSettings(void*)
{
  sasDebugOutput("saving input settings\n");
  sasDebugMng::singletonPtr()->serializeInputSettings();
}

void TW_CALL oceanCreatePreset(void*)
{
  sasDebugOutput("creating ocean preset\n");
  sasDebugMng::singletonPtr()->createOceanPreset();
}

void TW_CALL oceanDeletePreset(void*)
{
  sasDebugOutput("deleting ocean preset\n");
  sasDebugMng::singletonPtr()->deleteOceanPreset();
}

void TW_CALL oceanSavePreset(void*)
{
  sasDebugOutput("saving ocean presets\n");
  sasDebugMng::singletonPtr()->saveOceanPresets();
}

void TW_CALL oceanCancelPreset(void*)
{
  sasDebugOutput("discarding and reloading ocean preset\n");
  sasDebugMng::singletonPtr()->discardReloadOceanPresets();
}

void TW_CALL oceanEditPreset(void*)
{
  sasDebugOutput("editing ocean preset\n");
  sasDebugMng::singletonPtr()->editOceanPreset();
}

void TW_CALL oceanMakePresetCurrent(void*)
{
  sasDebugOutput("editing ocean preset\n");
  sasDebugMng::singletonPtr()->makeCurrentOceanPreset();
}


void sasDebugMng::createOceanPreset()
{
  
}

void sasDebugMng::deleteOceanPreset()
{
  
}

void sasDebugMng::saveOceanPresets()
{
  
}

void sasDebugMng::discardReloadOceanPresets()
{

}

void sasDebugMng::editOceanPreset()
{
  
}

void sasDebugMng::makeCurrentOceanPreset()
{
  
}

sasDebugMng::sasDebugMng()
  : _showDiffuseBuffer( false )
  , _showLightBuffer( false )
  , _showSpecBuffer( false )
  , _showNormalBuffer( false )
  , _showSSAOBuffer( false )
  , _showBoundingVolumes( false )
  , _showWireframe( false )
  , _enableCSLightPass( true )
  , _currentCSLightPassState( true )
  , _tessellationMode( 0 )
  , _disableSSAO( false )
  , _ssaoScale( 5.f )
  , _ssaoBias( 0.15f )
  , _ssaoIntensity( 20.f )
  , _ssaoDampen( 0.25f )
  , _motionBlurIntensity( 1.f )
  , _showVelocityBuffer( false )
  , _showShadowMap( 0 )
  , _showGodrayBuffer( false )
  , _showUVs( false )
  , _showDebugFrustum(false)
  , _takeFrustumShot(false)
  , _cascade0Range( 5.f )
  , _cascade1Range( 15.f )
  , _cascade2Range( 50.f )
  , _godraysIntensity( 1.2f )
  , _disableFXAA( false )
  , _sunIntensity( 5.f )
  , _editMode( false )
  , _controllerInverted( true )
  , _mouseInverted( false )
  , _lightCullingEnabled( true )
  , _volumetricLightDebugSlice( 0 )
  , _enableVolumetricLighting( true )
  , _newStyleRenderLists( true )
  , _enableGpuTimings( false )
  , _enableCpuTimings( false )
  , _vSyncEnabled( false )
  , _vrStereoEnabled( false )
  , _vrDistortionCompensation( false )
  , _vrEyeSeparation( 0.065f )
  , _vrLensSeparation( 0.f )
  , _forceGpuFlush( false )
  , _view2dFluidBuffer( false )
  , _view3dFluidBuffer( false )
  , _oceanLastFrameCurrentLayer( 0 )
  , _oceanCurrentLayer( 0 )
  , _oceanPresetEditMode( false )
{
  _disableTessellation = !sasSettings::singletonPtr()->_tessellationEnabled;
  _disableMotionBlur = !sasSettings::singletonPtr()->_motionBlurEnabled;
  _shadowsEnabled = sasSettings::singletonPtr()->_shadowsEnabled;
  _godraysEnabled = sasSettings::singletonPtr()->_godraysEnabled;
  _particlesEnabled = sasSettings::singletonPtr()->_particlesEnabled;
  _heightFogEnabled = sasSettings::singletonPtr()->_heightFogEnabled;

  loadInputSettingsFromDisk();

  // main debug bar
  _twBar = TwNewBar( "Debug" );
  TwAddButton( _twBar, "View buffers", &mainToBufferViewMenu, NULL, NULL );
  TwAddButton( _twBar, "Shadow menu", &mainToShadowMenu, NULL, NULL );
  TwAddButton( _twBar, "Sunlight menu", &mainToSunLightMenu, NULL, NULL );
  TwAddButton( _twBar, "Ssao menu", &mainToSsaoMenu, NULL, NULL );
  TwAddButton( _twBar, "Motion blur menu", &mainToMotionBlurMenu, NULL, NULL );
  TwAddButton( _twBar, "Ocean menu", &mainToOceanMenu, NULL, NULL );
  TwAddButton( _twBar, "Misc settings menu", &mainToMiscSettingsMenu, NULL, NULL );
  TwAddButton( _twBar, "Input menu", &mainToInputMenu, NULL, NULL );
  TwAddButton( _twBar, "VR menu", &mainToVRMenu, NULL, NULL );
  TwAddSeparator( _twBar, "Shaders", "" );
  TwAddButton( _twBar, "ReloadShaders", &reloadShaders, NULL, NULL );
  TwAddSeparator( _twBar, "Other", "" );
  TwAddVarRW( _twBar, "GPU timings", TW_TYPE_BOOLCPP, &_enableGpuTimings, "" );
  TwAddVarRW( _twBar, "CPU timings", TW_TYPE_BOOLCPP, &_enableCpuTimings, "" );

  TwDefine(" Debug position='20 350' ");

  // buffer view bar
  _twDebugViewBar = TwNewBar( "ViewBuffers" );
  TwAddButton( _twDebugViewBar, "Main menu", &viewBuffersToMainMenu, NULL, NULL );
  TwAddSeparator( _twDebugViewBar, "Debug views", "" );
  TwAddVarRW( _twDebugViewBar, "Show light buffer", TW_TYPE_BOOLCPP, &_showLightBuffer, "" );
  TwAddVarRW( _twDebugViewBar, "Show specular buffer", TW_TYPE_BOOLCPP, &_showSpecBuffer, "" );
  TwAddVarRW( _twDebugViewBar, "Show normal buffer", TW_TYPE_BOOLCPP, &_showNormalBuffer, "" );
  TwAddVarRW( _twDebugViewBar, "Show diffuse buffer", TW_TYPE_BOOLCPP, &_showDiffuseBuffer, "" );
  TwAddVarRW( _twDebugViewBar, "Show ssao buffer", TW_TYPE_BOOLCPP, &_showSSAOBuffer, "" );
  TwAddVarRW( _twDebugViewBar, "Show velocity buffer", TW_TYPE_BOOLCPP, &_showVelocityBuffer, "" );
  TwAddVarRW( _twDebugViewBar, "Show godrays", TW_TYPE_BOOLCPP, &_showGodrayBuffer, "" );
  TwAddVarRW( _twDebugViewBar, "Show wireframe", TW_TYPE_BOOLCPP, &_showWireframe, "" );
  TwAddVarRW( _twDebugViewBar, "Show bounding volumes", TW_TYPE_BOOLCPP, &_showBoundingVolumes, "" );
  TwAddVarRW( _twDebugViewBar, "Show UVs", TW_TYPE_BOOLCPP, &_showUVs, "" );
  TwAddVarRW( _twDebugViewBar, "Show shadowmap", TW_TYPE_UINT8, &_showShadowMap, "" );

  TwDefine(" ViewBuffers position='20 350' ");
  TwDefine(" ViewBuffers visible=false ");

  // misc settings bar
  _twDebugMiscBar = TwNewBar( "MiscSettings" );
  TwAddButton( _twDebugMiscBar, "Main menu", &miscSettingsToMainMenu, NULL, NULL );
  TwAddSeparator( _twDebugMiscBar, "Settings", "" );
  TwAddVarRW( _twDebugMiscBar, "newRenderLists", TW_TYPE_BOOLCPP, &_newStyleRenderLists, "" );
  TwAddVarRW( _twDebugMiscBar, "VSync", TW_TYPE_BOOLCPP, &_vSyncEnabled, "" );
  TwAddVarRW( _twDebugMiscBar, "Force GPU flush", TW_TYPE_BOOLCPP, &_forceGpuFlush, "" );
  TwAddVarRW( _twDebugMiscBar, "view 2d fluid", TW_TYPE_BOOLCPP, &_view2dFluidBuffer, "" );
  TwAddVarRW( _twDebugMiscBar, "view 3d fluid", TW_TYPE_BOOLCPP, &_view3dFluidBuffer, "" );
  TwAddVarRW( _twDebugMiscBar, "heightFog", TW_TYPE_BOOLCPP, &_heightFogEnabled, "" );
  TwAddVarRW( _twDebugMiscBar, "particles", TW_TYPE_BOOLCPP, &_particlesEnabled, "" );
  TwAddSeparator( _twDebugMiscBar, "Lighting", "" );
  TwAddVarRW( _twDebugMiscBar, "Enable CS lightpass", TW_TYPE_BOOLCPP, &_enableCSLightPass, "" );
  TwAddVarRW( _twDebugMiscBar, "light culling", TW_TYPE_BOOLCPP, &_lightCullingEnabled, "" );
  TwAddVarRW( _twDebugMiscBar, "Enable vol lighting", TW_TYPE_BOOLCPP, &_enableVolumetricLighting, "" );
  TwAddVarRW( _twDebugMiscBar, "debug light vol slice", TW_TYPE_INT32, &_volumetricLightDebugSlice, " min=0.0 " );
  TwAddSeparator( _twDebugMiscBar, "Tessellation", "" );
  TwAddVarRW( _twDebugMiscBar, "Disable tessellation", TW_TYPE_BOOLCPP, &_disableTessellation, "" );
  TwAddVarRW( _twDebugMiscBar, "Tessellation mode", TW_TYPE_UINT8, &_tessellationMode, "" );
  TwAddSeparator( _twDebugMiscBar, "AA", "" );
  TwAddVarRW( _twDebugMiscBar, "Disable FXAA", TW_TYPE_BOOLCPP, &_disableFXAA, "" );
  TwAddSeparator( _twDebugMiscBar, "Frustum", "" );
  TwAddButton( _twDebugMiscBar, "Take frustum shot", &captureFrustumTw, NULL, NULL );
  TwAddVarRW( _twDebugMiscBar, "debug frustum", TW_TYPE_BOOLCPP, &_showDebugFrustum, "" );

  TwDefine(" MiscSettings position='20 350' ");
  TwDefine(" MiscSettings visible=false ");

  // ssao bar
  _twSSAOBar = TwNewBar( "SSAO" );
  TwAddButton( _twSSAOBar, "Main menu", &ssaoToMainMenu, NULL, NULL );
  TwAddSeparator( _twSSAOBar, "SSAO", "" );
  TwAddVarRW( _twSSAOBar, "Disable ssao", TW_TYPE_BOOLCPP, &_disableSSAO, "" );
  TwAddVarRW( _twSSAOBar, "Show ssao buffer", TW_TYPE_BOOLCPP, &_showSSAOBuffer, "" );
  TwAddVarRW( _twSSAOBar, "ssao scale", TW_TYPE_FLOAT, &_ssaoScale, " min=0.0 max=30.0 step=0.1 " );
  TwAddVarRW( _twSSAOBar, "ssao bias", TW_TYPE_FLOAT, &_ssaoBias, " min=-1.0 max=1.0 step=0.05 " );
  TwAddVarRW( _twSSAOBar, "ssao intensity", TW_TYPE_FLOAT, &_ssaoIntensity, " min=0.0 max=20.0 step=0.1 " );
  TwAddVarRW( _twSSAOBar, "ssao dampen", TW_TYPE_FLOAT, &_ssaoDampen, " min=0.0 max=4.0 step=0.05 " );

  TwDefine(" SSAO position='20 350' ");
  TwDefine(" SSAO visible=false ");

  // motion blur bar
  _twMotionBlurBar = TwNewBar( "MotionBlur" );
  TwAddButton( _twMotionBlurBar, "Main menu", &motionBlurToMainMenu, NULL, NULL );
  TwAddSeparator( _twMotionBlurBar, "Motion blur", "" );
  TwAddVarRW( _twMotionBlurBar, "Show velocity buffer", TW_TYPE_BOOLCPP, &_showVelocityBuffer, "" );
  TwAddVarRW( _twMotionBlurBar, "Disable motion blur", TW_TYPE_BOOLCPP, &_disableMotionBlur, "" );
  TwAddVarRW( _twMotionBlurBar, "Motion blur intensity", TW_TYPE_FLOAT, &_motionBlurIntensity, " min=0.0 max=30.0 step=0.1 " );
  
  TwDefine(" MotionBlur position='20 350' ");
  TwDefine(" MotionBlur visible=false "); 

  // shadow debug bar
  _twShadowBar = TwNewBar( "Shadows" );
  TwAddButton( _twShadowBar, "Main menu", &shadowToMainMenu, NULL, NULL );
  TwAddSeparator( _twShadowBar, "Debug options", "" );
  TwAddVarRW( _twShadowBar, "Shadows enabled", TW_TYPE_BOOLCPP, &_shadowsEnabled, "" ); 
  TwAddVarRW( _twShadowBar, "Show shadowmap", TW_TYPE_UINT8, &_showShadowMap, "" );
  TwAddVarRW( _twShadowBar, "c0 range", TW_TYPE_FLOAT, &_cascade0Range, " min=1.0 max=50.0 step=0.1 " );
  TwAddVarRW( _twShadowBar, "c1 range", TW_TYPE_FLOAT, &_cascade1Range, " min=1.0 max=100.0 step=0.1 " );
  TwAddVarRW( _twShadowBar, "c2 range", TW_TYPE_FLOAT, &_cascade2Range, " min=1.0 max=200.0 step=0.1 " );
  TwAddVarRW( _twShadowBar, "Godrays enabled", TW_TYPE_BOOLCPP, &_godraysEnabled, "" ); 
  TwAddVarRW( _twShadowBar, "Godrays intensity", TW_TYPE_FLOAT, &_godraysIntensity, " min=0.0 max=10.0 step=0.1 " );
  TwAddVarRW( _twShadowBar, "Show godrays", TW_TYPE_BOOLCPP, &_showGodrayBuffer, "" );

  TwDefine(" Shadows position='20 350' ");
  TwDefine(" Shadows visible=false ");

  // directional light debug bar
  _sunColour[ 0 ] = 0.28f; _sunColour[ 1 ] = 0.3f; _sunColour[ 2 ] = 0.5f;
  _sunDir[ 0 ] = 0.5f; _sunDir[ 1 ] = -0.8f; _sunDir[ 2 ] = 0.15f;
  _twSunlightBar = TwNewBar( "Sunlight" );
  TwAddButton( _twSunlightBar, "Main menu", &sunLightToMainMenu, NULL, NULL );
  TwAddSeparator( _twSunlightBar, "Debug options", "" );
  TwAddVarRW( _twSunlightBar, "Sun colour", TW_TYPE_COLOR3F, &_sunColour, "" ); 
  TwAddVarRW( _twSunlightBar, "Sun intensity", TW_TYPE_FLOAT, &_sunIntensity, " min=1.0 max=5000.0 step=0.1 " );
  TwAddVarRW( _twSunlightBar, "Sun direction", TW_TYPE_DIR3F, &_sunDir, "" );

  TwDefine(" Sunlight position='20 350' ");
  TwDefine(" Sunlight visible=false ");

  // input bar
  _twInputBar = TwNewBar( "Input" );
  TwAddButton( _twInputBar, "Main menu", &inputToMainMenu, NULL, NULL );
  TwAddSeparator( _twInputBar, "Options", "" );
  TwAddVarRW( _twInputBar, "PadY inverted", TW_TYPE_BOOLCPP, &_controllerInverted, "" ); 
  TwAddVarRW( _twInputBar, "MouseY inverted", TW_TYPE_BOOLCPP, &_mouseInverted, "" );
  TwAddButton( _twInputBar, "Save settings", &saveInputSettings, NULL, NULL );

  TwDefine(" Input position='20 350' ");
  TwDefine(" Input visible=false ");


  // vr bar
  _twVRBar = TwNewBar( "VR" );
  TwAddButton( _twVRBar, "Main menu", &vrToMainMenu, NULL, NULL );
  TwAddSeparator( _twVRBar, "Options", "" );
  TwAddVarRW( _twVRBar, "Stereo enabled", TW_TYPE_BOOLCPP, &_vrStereoEnabled, "" ); 
  TwAddVarRW( _twVRBar, "Distortion comp.", TW_TYPE_BOOLCPP, &_vrDistortionCompensation, "" );
  TwAddVarRW( _twVRBar, "Eye dist", TW_TYPE_FLOAT, &_vrEyeSeparation, " min=0.0 max=5.2 step=0.001 " );
  TwAddVarRW( _twVRBar, "Lens dist", TW_TYPE_FLOAT, &_vrLensSeparation, " min=-250.0 max=5000.0 step=0.001 " );
  TwAddVarRW( _twVRBar, "Override FOV", TW_TYPE_BOOLCPP, &_vrOverrideFov, "" );
  TwAddVarRW( _twVRBar, "Fov vertical", TW_TYPE_BOOLCPP, &_vrFovVertical, "" );

  TwDefine(" VR position='20 350' ");
  TwDefine(" VR visible=false ");

  //debug frustum render resources
  sasVertexFormat frustumVertexFmt;
  frustumVertexFmt.position = sasVertexFormat::POSITION_3F;
  _frustumVB = sasNew sasVertexBuffer( frustumVertexFmt, 36 );
}

sasDebugMng::~sasDebugMng()
{
  sasDelete _frustumVB;
  TwDeleteBar( _twVRBar );
  TwDeleteBar( _twInputBar );
  TwDeleteBar( _twDebugViewBar );
  TwDeleteBar( _twDebugMiscBar );
  TwDeleteBar( _twSSAOBar );
  TwDeleteBar( _twMotionBlurBar );
  TwDeleteBar( _twSunlightBar );
  TwDeleteBar( _twShadowBar );
  TwDeleteBar( _twBar );
}

void sasDebugMng::setMainMenuVisibility( bool state )
{
  if( state )
    TwDefine(" Debug visible=true ");
  else
    TwDefine(" Debug visible=false ");
}

void sasDebugMng::setShadowMenuVisibility( bool state )
{
  if( state )
    TwDefine(" Shadows visible=true ");
  else
    TwDefine(" Shadows visible=false ");
}

void sasDebugMng::setSunLightMenuVisibility( bool state )
{
  if( state )
    TwDefine(" Sunlight visible=true ");
  else
    TwDefine(" Sunlight visible=false ");
}

void sasDebugMng::setDebugMiscMenuVisibility( bool state )
{
  if( state )
    TwDefine(" MiscSettings visible=true ");
  else
    TwDefine(" MiscSettings visible=false ");
}

void sasDebugMng::setMotionBlurMenuVisibility( bool state )
{
  if( state )
    TwDefine(" MotionBlur visible=true ");
  else
    TwDefine(" MotionBlur visible=false ");
}

void sasDebugMng::setOceanPresetMenuVisibility( bool state )
{
  _oceanPresetEditMode = state;
  if( state )
  {
    TwDefine(" OceanPreset visible=true ");
    TwDefine(" Ocean visible=false ");
  }
  else
  {
    _oceanPresetName = "no_name";
    TwDefine(" OceanPreset visible=false ");
    TwDefine(" Ocean visible=true ");
  }
}

void sasDebugMng::setOceanMenuVisibility( bool state )
{
  if( state )
    TwDefine(" Ocean visible=true ");
  else
    TwDefine(" Ocean visible=false ");
}

void sasDebugMng::setSsaoMenuVisibility( bool state )
{
  if( state )
    TwDefine(" SSAO visible=true ");
  else
    TwDefine(" SSAO visible=false ");
}

void sasDebugMng::setDebugViewMenuVisibility( bool state )
{
  if( state )
    TwDefine(" ViewBuffers visible=true ");
  else
    TwDefine(" ViewBuffers visible=false ");
}

void sasDebugMng::setInputMenuVisibility( bool state )
{
  if( state )
    TwDefine(" Input visible=true ");
  else
    TwDefine(" Input visible=false ");
}

void sasDebugMng::setVRMenuVisibility( bool state )
{
  if( state )
    TwDefine(" VR visible=true ");
  else
    TwDefine(" VR visible=false ");
}

void sasDebugMng::applyDebugOptions( sasRenderTarget* diffuseBuffer, sasRenderTarget* normalBuffer, sasRenderTarget* lightBuffer, sasRenderTarget* specBuffer, 
                                      sasRenderTarget* ssaoBuffer, sasRenderTarget* velocityBuffer, sasRenderTarget* volLightBuffer, sasRenderTarget* destBuffer )
{
  sasGpuProfiler::singletonPtr()->setEnabled( _enableGpuTimings );
  sasDevice::singletonPtr()->setVSyncEnabled( _vSyncEnabled );
  sasDevice::singletonPtr()->setForceGPUFlush( _forceGpuFlush );

  //update vr settings
  sasStereo::singletonPtr()->setEnabled( _vrStereoEnabled );
  sasStereo::singletonPtr()->setEyeSeparation( _vrEyeSeparation );
  sasStereo::singletonPtr()->setDistortionCompPassEnabled( _vrDistortionCompensation );
  sasStereo::singletonPtr()->setLensSeparation( _vrLensSeparation );
  sasStereo::singletonPtr()->setOverrideFov( _vrOverrideFov );
  sasStereo::singletonPtr()->setVerticalFov( _vrFovVertical );

  //update editor settings
  sasEditor::singletonPtr()->setEnabled( _editMode );

  //update light settings
  sasLightMng::singletonPtr()->setSunColour( smVec3( _sunColour[ 0 ], _sunColour[ 1 ], _sunColour[ 2 ] ) * _sunIntensity );
  sasLightMng::singletonPtr()->setSunDir( smVec3( _sunDir[ 0 ], _sunDir[ 1 ], _sunDir[ 2 ] ) );

  //update shadow settings
  if( _shadowsEnabled )
  {
    smVec4 cascadeDistances( _cascade0Range, _cascade1Range, _cascade2Range, 0.f );
    sasShadowMng::singletonPtr()->setCascadeRanges( cascadeDistances );
    sasShadowMng::singletonPtr()->setGodraysIntensity( _godraysIntensity );
    if( !_godraysEnabled && sasSettings::singletonPtr()->_godraysEnabled )
    {
      sasShadowMng::singletonPtr()->clearGodrayBuffer();
    }
  }
  else if( sasSettings::singletonPtr()->_shadowsEnabled == true )
  {
    //if shadows have just been disabled this frame
    sasShadowMng::singletonPtr()->clearScreenSpaceShadowMap();
    sasShadowMng::singletonPtr()->clearGodrayBuffer();
  }

  //Update render settings
  sasSettings::singletonPtr()->_tessellationEnabled = !_disableTessellation;
  sasSettings::singletonPtr()->_tessellationMode = static_cast<sasTessellationSmoothing::Enum>(_tessellationMode);
  sasSettings::singletonPtr()->_ssaoEnabled = !_disableSSAO;
  sasSettings::singletonPtr()->_motionBlurEnabled = !_disableMotionBlur;
  sasSettings::singletonPtr()->_motionBlurIntensity = _motionBlurIntensity;
  sasSettings::singletonPtr()->_shadowsEnabled = _shadowsEnabled;
  sasSettings::singletonPtr()->_godraysEnabled = _godraysEnabled;
  sasSettings::singletonPtr()->_fxaaEnabled = !_disableFXAA;
  sasSettings::singletonPtr()->_volumetricLightingEnabled = _enableVolumetricLighting;
  sasSettings::singletonPtr()->_particlesEnabled = _particlesEnabled;
  sasSettings::singletonPtr()->_heightFogEnabled = _heightFogEnabled;

  if( _currentCSLightPassState != _enableCSLightPass )
  {
    _currentCSLightPassState = _enableCSLightPass;
    sasRenderMng::singletonPtr()->setCSLightPassState(_enableCSLightPass);
  }
  
  //Display debug render targets
  if( !( _showDiffuseBuffer | _showLightBuffer | _showNormalBuffer | _showSpecBuffer | _showSSAOBuffer | _showVelocityBuffer | 
        !!_showShadowMap | _showGodrayBuffer | !!_volumetricLightDebugSlice ) )
    return;

  if( _showSpecBuffer ) 
  {
    sasRenderMng::singletonPtr()->copyRenderTarget( specBuffer, destBuffer );
    return;
  }

  if( _showNormalBuffer )
  {
    sasRenderMng::singletonPtr()->copyRenderTarget( normalBuffer, destBuffer );
    return;
  }

  if( _showDiffuseBuffer )
  {
    sasRenderMng::singletonPtr()->copyRenderTarget( diffuseBuffer, destBuffer );
    return;
  }

  if( _showLightBuffer )
  {
    sasRenderMng::singletonPtr()->copyRenderTarget( lightBuffer, destBuffer );
    return;
  }

  if( _showSSAOBuffer )
  {
    sasRenderMng::singletonPtr()->copyRenderTarget( ssaoBuffer, destBuffer );
    return;
  }

  if( _showVelocityBuffer )
  {
    sasRenderMng::singletonPtr()->copyRenderTarget( velocityBuffer, destBuffer );
    return;
  }

  if( _showShadowMap > 0 && _showShadowMap < 4)
  {
    sasRenderMng::singletonPtr()->copyDepthTargetToRenderTarget( sasShadowMng::singletonPtr()->shadowMap( _showShadowMap - 1 ), destBuffer );
    return;
  }

  if( _showGodrayBuffer )
  {
    sasRenderMng::singletonPtr()->copyRenderTarget( sasShadowMng::singletonPtr()->godRayBuffer(), destBuffer );
    return;
  }

  if( _volumetricLightDebugSlice > 0 )
  {
    sasDebugRender::singletonPtr()->renderVolumeTextureSlice( volLightBuffer, _volumetricLightDebugSlice - 1 );
  }
}

void sasDebugMng::renderFrustum( sasDevice& device, const sasFrustumPoints& frustum )
{
  //Render frustum
  smVec3 pos[] = {  
    smVec3( frustum.getPoint(sasFrustumPoints::nearTopRight).X, frustum.getPoint(sasFrustumPoints::nearTopRight).Y, frustum.getPoint(sasFrustumPoints::nearTopRight).Z ), // ntr
    smVec3( frustum.getPoint(sasFrustumPoints::nearTopLeft).X, frustum.getPoint(sasFrustumPoints::nearTopLeft).Y, frustum.getPoint(sasFrustumPoints::nearTopLeft).Z ), // ntl
    smVec3( frustum.getPoint(sasFrustumPoints::nearBottomLeft).X, frustum.getPoint(sasFrustumPoints::nearBottomLeft).Y, frustum.getPoint(sasFrustumPoints::nearBottomLeft).Z ), // nbl
    smVec3( frustum.getPoint(sasFrustumPoints::nearBottomRight).X, frustum.getPoint(sasFrustumPoints::nearBottomRight).Y, frustum.getPoint(sasFrustumPoints::nearBottomRight).Z ), // nbr
    smVec3( frustum.getPoint(sasFrustumPoints::farBottomRight).X, frustum.getPoint(sasFrustumPoints::farBottomRight).Y, frustum.getPoint(sasFrustumPoints::farBottomRight).Z ), // fbr
    smVec3( frustum.getPoint(sasFrustumPoints::farTopRight).X, frustum.getPoint(sasFrustumPoints::farTopRight).Y, frustum.getPoint(sasFrustumPoints::farTopRight).Z ), // ftr
    smVec3( frustum.getPoint(sasFrustumPoints::farTopLeft).X, frustum.getPoint(sasFrustumPoints::farTopLeft).Y, frustum.getPoint(sasFrustumPoints::farTopLeft).Z ), // ftl
    smVec3( frustum.getPoint(sasFrustumPoints::farBottomLeft).X, frustum.getPoint(sasFrustumPoints::farBottomLeft).Y, frustum.getPoint(sasFrustumPoints::farBottomLeft).Z ), // fbl
  };

  const uint16_t indices[] = { 0, 2, 1,   0, 3, 2,  // Front
    1, 7, 6,   1, 2, 7,  // Left
    5, 3, 0,   5, 4, 3,  // Right
    6, 0, 1,   6, 5, 0,  // Top
    3, 7, 2,   3, 4, 7,  // Bottom
    5, 6, 7,   5, 7, 4,  // Back
  };

  const size_t nrOfIndices = sizeof(indices) / sizeof(indices[0]);

  float* posData = (float*)_frustumVB->map( false );
  if( posData != NULL)
  {
    for( size_t i = 0; i < nrOfIndices; i++ )
    {
      memcpy(posData, (const float*)&pos[indices[i]], sizeof(float) * 3);
      posData += 3;
    }
    _frustumVB->unmap();
  }
  
  device.setVertexBuffer( _frustumVB );
  device.setIndexBuffer( sasMiscResources::singletonPtr()->getUnitCubeIB() );
  device.setVertexShader( renderFrustumShaderID );
  device.setGeometryShader( renderFrustumShaderID );
  device.setPixelShader( renderFrustumShaderID );
  device.setRasterizerState( sasRasterizerState_Solid_NoCull );
  device.setBlendState( sasBlendState_One_One );
  device.setDepthStencilState( sasDepthStencilState_LessEqual, 0 );
  device.draw( sasPrimTopology_TriList, _frustumVB->verticesCount() );

  device.setBlendState( sasBlendState_Opaque );
  device.setRasterizerState( sasRasterizerState_Solid );

  sasShaderID nullShaderID;
  device.setGeometryShader( nullShaderID );
}

void sasDebugMng::renderLights()
{
  sasLightMng* lightMng = sasLightMng::singletonPtr();
  sasDebugRender* debugRender = sasDebugRender::singletonPtr();
  std::vector< sasPointLight* > pointLightList = lightMng->pointLightArray();
  for( size_t i = 0; i < pointLightList.size(); i++ )
  {
    debugRender->renderSphere( pointLightList[ i ]->position(), pointLightList[ i ]->radius() * 0.05f, pointLightList[ i ]->colour() );
  }

  std::vector< sasSpotLight* > spotLightList = lightMng->spotLightArray();
  for( size_t i = 0; i < spotLightList.size(); i++ )
  {
    debugRender->renderSphere( spotLightList[ i ]->position(), spotLightList[ i ]->radius() * 0.05f, spotLightList[ i ]->colour() );
  }
}

void sasDebugMng::renderParticles()
{
}

void sasDebugMng::renderDebugObjects( sasCamera* camera, sasRenderTarget* rt, sasDepthStencilTarget* depthBuffer )
{
  static sasFrustumPoints cachedFrustumPoints;

  sasDevice* device = sasDevice::singletonPtr();
  device->setRenderTarget( 0, rt );
  device->setDepthStencilTarget( depthBuffer, false );

  if( takeFrustumShot() )
  {
    cachedFrustumPoints = sasFrustumPoints( *camera, (float)device->backBufferWidth(), (float)device->backBufferHeight() );
    frustumShotTaken();
  }
  if( showDebugFrustum() )
  {
    renderFrustum( *device, cachedFrustumPoints );
  }

  device->setDepthStencilTarget( NULL, false );
  device->setRenderTarget( 0, device->backBuffer() );
}


void sasDebugMng::serializeInputSettings()
{
  sasFile isFile;
  std::string path = sasResourceMng::singletonPtr()->resourcePath();
  path += "inputSettings.sasfile";
  if( !isFile.open( path.c_str(), sasFile::Write ) )
  {
    return;
  }

  isFile.write( kInputDataVersion );

  isFile.write( _controllerInverted );
  isFile.write( _mouseInverted );
}

bool sasDebugMng::loadInputSettingsFromDisk()
{
  sasFile isFile;
  std::string path = sasResourceMng::singletonPtr()->resourcePath();
  path += "inputSettings.sasfile";
  if( !isFile.open( path.c_str(), sasFile::Read ) )
  {
    sasDebugOutput("Couldn't open input settings data file '%s'\n", path.c_str() );
    return false;
  }

  uint32_t version;
  isFile.read( version );
  sasASSERT( version == kInputDataVersion );

  isFile.read( _controllerInverted );
  isFile.read( _mouseInverted );

  return true;
}