#include "sas_pch.h"
#include "sas_shadermng.h"
#include "sas_device_factory.h"
#include "sas_shaderflags.h"
#include "sas_shaderfamily.h"
#include "sas_shaderids.h"


sasShaderMng::sasShaderMng( const char* shaderFolder )
  : _shaderFolder( shaderFolder )
  , _globalFlags(0)
  , _families(0)
  , _nFamilies(0)
{
  loadSetup();
}

sasShaderMng::~sasShaderMng()
{  
  releaseShaders();
  sasDelete _globalFlags;
  for( size_t i=0; i<_nFamilies; i++ )
    sasDelete _families[i].family;
  sasFree( _families ); 
}

sasShaderID sasShaderMng::getFamilyBitFlag( const sasStringId& key )
{
  sasShaderID mask;
  for( size_t i=0; i<_nFamilies; i++ )
  {
    if( _families[i].key == key )
    {
      mask.flags = 0;
      mask.family = i;      
      break;
    }
  }
  return mask;
}

sasShaderID sasShaderMng::getOptionBitMask( sasShaderID id, const sasStringId& opt, size_t value )
{
  sasASSERT( id.family < _nFamilies );
  sasShaderFamily* family = _families[ id.family ].family;  
  return family->getOptionBitMask( opt, value );
}

sasShaderID sasShaderMng::getGlobalOptionBitMask( const sasStringId& opt, size_t value )
{
  sasASSERT( _globalFlags );  
  return _globalFlags->getOptionBitMask( opt, value );
}

sasDeviceObject* sasShaderMng::getVertexShader( sasShaderID id )
{
  return getShader( id, sasShaderType::Vertex );  
}

sasDeviceObject* sasShaderMng::getHullShader( sasShaderID id )
{
  return getShader( id, sasShaderType::Hull );  
}

sasDeviceObject* sasShaderMng::getDomainShader( sasShaderID id )
{
  return getShader( id, sasShaderType::Domain );  
}

sasDeviceObject* sasShaderMng::getGeometryShader( sasShaderID id )
{
  return getShader( id, sasShaderType::Geometry );  
}

sasDeviceObject* sasShaderMng::getPixelShader( sasShaderID id )
{
  return getShader( id, sasShaderType::Pixel );  
}

sasDeviceObject* sasShaderMng::getComputeShader( sasShaderID id )
{
  return getShader( id, sasShaderType::Compute );
}

void sasShaderMng::loadSetup()
{
  std::string path = _shaderFolder;
  path += "globals.txt";
  json_t* root;
  json_error_t error;

  root = json_load_file( path.c_str(), 0, &error);
  sasASSERT( root );
  loadGlobalFlags( root );
  loadFamilies( root );
  json_decref( root );
}

void sasShaderMng::loadGlobalFlags( json_t* root )
{
  json_t* shaderFlags = json_object_get(root, "ShaderFlags" );
  if( shaderFlags && json_is_object( shaderFlags ) )
  {    
    _globalFlags = sasNew sasShaderFlags( shaderFlags );      
  } 
}

void sasShaderMng::loadFamilies( json_t* root )
{ 
  json_t* families = json_object_get(root, "Families" );
  sasASSERT( families && json_is_array(families) );
  _nFamilies = json_array_size( families );    
  sasASSERT( _nFamilies>0 );  
  size_t nBytes = _nFamilies * sizeof(Family);
  _families = (Family*)sasMalloc( nBytes );
  memset( _families, 0, nBytes );
  for( size_t i=0; i<_nFamilies; i++ )
  {
    json_t* familyNode = json_array_get( families, i );
    sasASSERT( familyNode && json_is_string(familyNode) );
    const char* familyName = json_string_value( familyNode );
    sasShaderFamily* family = getFamily( sasStringId::build( familyName ) );
    sasASSERT( !family );
    family = loadFamily( familyName );
    _families[ i ].family = family;
    _families[ i ].key = sasStringId::build( familyName );
  }
}

sasShaderFamily* sasShaderMng::loadFamily( const char* familyName )
{
  sasShaderFamily* result = 0;
  std::string path = _shaderFolder;
  path += familyName;
  path += ".txt";

  json_t* root;
  json_error_t error;
  root = json_load_file( path.c_str(), 0, &error);
  sasASSERT( root );
  result = sasNew sasShaderFamily( root );
  json_decref( root );
  return result;
}

sasShaderFamily* sasShaderMng::getFamily( const sasStringId& id ) const
{
  for( size_t i=0; i<_nFamilies; i++ )
  {
    if( _families[i].key == id )
      return _families[i].family;
  }
  return 0;
}

sasDeviceObject* sasShaderMng::getShader( sasShaderID id, sasShaderType::Enum type )
{
  sasASSERT( id.isValid() && id.family < _nFamilies );
  sasShaderFamily* family = _families[ id.family ].family;  
  return family != NULL ? family->getShader( id, type ) : NULL;
}

void sasShaderMng::releaseShaders()
{
  for( size_t i=0; i<_nFamilies; i++ )
    _families[i].family->releaseShaders();
}

void sasShaderMng::initializeShaderIds()
{
  defaultShaderID =         getFamilyBitFlag( "default" );
  screenCopyShaderID =      getFamilyBitFlag("screenCopy");
  compositePassShaderID =   getFamilyBitFlag("compositePass");
  lightPassShaderID =       getFamilyBitFlag("lightPass");
  deferredShadowsShaderID = getFamilyBitFlag("deferredShadows");
  godraysShaderID =         getFamilyBitFlag( "godrays" );
  lightPassNoCSShaderID =   getFamilyBitFlag( "lightPassNoCS" );
  ssaoShaderID =            getFamilyBitFlag( "ssao" );
  bilateralBlurShaderID =   getFamilyBitFlag( "bilateralBlur" );
  volumetricLightShaderID = getFamilyBitFlag( "volumetricLight" );
  worldSpacePosShaderID =   getFamilyBitFlag( "worldSpacePos" );
  motionBlurShaderID =      getFamilyBitFlag( "motionBlur" );
  renderFrustumShaderID =   getFamilyBitFlag( "renderFrustum" );
  gatherParticlesShaderID = getFamilyBitFlag( "gatherParticles" );
  renderParticlesShaderID = getFamilyBitFlag( "destrParticles" );
  renderLineShaderID =      getFamilyBitFlag( "renderLine" );
  sphShaderID =             getFamilyBitFlag( "sphParticles" );
  fxaaShaderID =            getFamilyBitFlag( "fxaa" );
  meshPickingShaderID =     getFamilyBitFlag( "meshPickingPass" );
  genVolLightPassShaderID = getFamilyBitFlag( "genVolLightPass" );
  ovrDistortionShaderID =   getFamilyBitFlag( "ovrDistortion" );
  gpuParticlesSimID =       getFamilyBitFlag( "gpuParticles" );
  gpuParticlesRenderID =    getFamilyBitFlag( "gpuPartRender" );
  fluidSim2DShaderID =		  getFamilyBitFlag( "fluidSim2D" );
  fluidSim3DShaderID =		  getFamilyBitFlag( "fluidSim3D" );
  heightFogShaderID =       getFamilyBitFlag( "heightFog" );
  depthDownsampleShaderID = getFamilyBitFlag( "depthDownsample" );
  dofShaderID             = getFamilyBitFlag( "dof" );
  ssrShaderID             = getFamilyBitFlag( "ssr" );
  oceanShaderID           = getFamilyBitFlag( "ocean" );
  sandShaderID            = getFamilyBitFlag( "sand" );
  render3DTexSliceShaderID =        getFamilyBitFlag( "debugVolTexSlice" );
  renderIndirectParticlesShaderID = getFamilyBitFlag( "renderParticles" );

  depthMask =                       getOptionBitMask( defaultShaderID, "DEPTH" );
  debugDiffuseOnlyMask =            getOptionBitMask( defaultShaderID, "DEBUG_DIFFUSE" );
  diffuseMask =                     getOptionBitMask( defaultShaderID, "DIFFUSE_MAP", 1 ); 
  debugUVsMask =                    getOptionBitMask( defaultShaderID, "DEBUG_UVS", 1 ); 
  normalMapMask =                   getOptionBitMask( defaultShaderID, "NORMAL_MAP", 1 ); 
  specMapMask =                     getOptionBitMask( defaultShaderID, "SPEC_MAP", 1 ); 
  maskMapMask =                     getOptionBitMask( defaultShaderID, "MASK_MAP", 1 ); 
  heightMapMask =                   getOptionBitMask( defaultShaderID, "HEIGHT_MAP", 1 ); 
  hasTangentMask =                  getOptionBitMask( defaultShaderID, "HAS_TANGENT", 1 ); 
  hasUVsMask =                      getOptionBitMask( defaultShaderID, "HAS_UVS", 1 ); 
  hasBoneDataMask =                 getOptionBitMask( defaultShaderID, "HAS_BONE_DATA", 1 ); 
  hasInstancedXfms =                getOptionBitMask( defaultShaderID, "INSTANCED_XFMS", 1 );
  velocityOutputMask =              getOptionBitMask( defaultShaderID, "OUTPUT_VELOCITY", 1 ); 
  cubemapGSOutputMask =             getOptionBitMask( defaultShaderID, "GS_CUBE_OUTPUT", 1 ); 
  cubeReflectionMask =              getOptionBitMask( defaultShaderID, "CUBE_MAP", 1 ); 
  tessellationMask =                getOptionBitMask( defaultShaderID, "TESSELLATION", 1);
  tessellationPhongSmoothingMask =  getOptionBitMask( defaultShaderID, "PHONG_SMOOTHING", 1 );
  tessellationPNTSmoothingMask =    getOptionBitMask( defaultShaderID, "PNT_SMOOTHING", 1 );
  geomShaderMask =                  getOptionBitMask( defaultShaderID, "GEOM_SHADER", 1 );
  meshIdMask =                      getOptionBitMask( defaultShaderID, "MESH_ID", 1 );
  godraysBlurMask =                 getOptionBitMask( godraysShaderID, "BLUR_PASS" );
  godraysLightScatterMask =         getOptionBitMask( godraysShaderID, "LLIGHT_SCATTER" ); 
  quadOverrideMask =                getOptionBitMask( screenCopyShaderID, "QUAD_OVERRIDE" );
  quadColXfm =                      getOptionBitMask( screenCopyShaderID, "COL_RESCALE" );
  quadFlatColTint =                 getOptionBitMask( screenCopyShaderID, "FLAT_COL" );
  lightPassStereoLeftEyeMask =      getOptionBitMask( lightPassShaderID, "STEREO_LEFT_EYE" );
  lightPassStereoRightEyeMask =     getOptionBitMask( lightPassShaderID, "STEREO_RIGHT_EYE" );
  stereoMask =                      getGlobalOptionBitMask( "STEREO" );
}
