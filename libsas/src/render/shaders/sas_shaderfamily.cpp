#include "sas_pch.h"
#include "sas_shaderfamily.h"
#include "sas_shadermng.h"
#include "sas_shaderflags.h"
#include "sas_device_factory.h"

sasShaderFamily::sasShaderFamily( json_t* root )
  : _shaderFlags(0)
{
  loadFileName( root );
  loadEntryPoints( root );
  loadShaderFlags( root );
}

sasShaderFamily::~sasShaderFamily()
{
  sasDelete _shaderFlags;
}

sasShaderID sasShaderFamily::getOptionBitMask( const sasStringId& opt, size_t value )
{
  sasASSERT( _shaderFlags );  
  return _shaderFlags->getOptionBitMask( opt, value );
}

sasDeviceObject* sasShaderFamily::getShader( sasShaderID id, sasShaderType::Enum type )
{
  switch( type )
  {
  case sasShaderType::Vertex: return getShader( id, type, _vertexShaders ); break;
  case sasShaderType::Hull:   return getShader( id, type, _hullShaders ); break;
  case sasShaderType::Domain: return getShader( id, type, _domainShaders ); break;
  case sasShaderType::Geometry: return getShader( id, type, _geometryShaders ); break;
  case sasShaderType::Pixel:  return getShader( id, type, _pixelShaders ); break;
  case sasShaderType::Compute: return getShader( id, type, _computeShaders ); break;
  }
  sasASSERT( 0 );
  return 0;
}

void sasShaderFamily::releaseShaders()
{
  releaseShaders( _vertexShaders );
  releaseShaders( _hullShaders );
  releaseShaders( _domainShaders );
  releaseShaders( _geometryShaders );
  releaseShaders( _pixelShaders );
  releaseShaders( _computeShaders );
}

void sasShaderFamily::loadFileName( json_t* root )
{
  std::string key = "file_";
  
  key += sasBACKEND;

  json_t* jsonFileName = json_object_get( root, key.c_str() );  
  sasASSERT( json_is_string( jsonFileName ) );
  _fileName = json_string_value( jsonFileName );
}

void sasShaderFamily::loadEntryPoints( json_t* root )
{
  bool bValid = false;
  // vs is mandatory! //Fred: no it isn't! :p
  {
    const char* func = "func_vs";    
    json_t* jsonFunc = json_object_get( root, func );  
    if( jsonFunc )
    {
      sasASSERT( json_is_string( jsonFunc ) );
      bValid = true;
      _entryPointVS = json_string_value( jsonFunc );
    }
  }

  // hs
  {
    const char* func = "func_hs";    
    json_t* jsonFunc = json_object_get( root, func );  
    if( jsonFunc )
    {
      sasASSERT( json_is_string( jsonFunc ) );
      _entryPointHS = json_string_value( jsonFunc );
    }    
  }

  // ds
  {
    const char* func = "func_ds";    
    json_t* jsonFunc = json_object_get( root, func );  
    if( jsonFunc )
    {
      sasASSERT( json_is_string( jsonFunc ) );
      _entryPointDS = json_string_value( jsonFunc );
    }    
  }

  // gs
  {
    const char* func = "func_gs";    
    json_t* jsonFunc = json_object_get( root, func );  
    if( jsonFunc )
    {
      sasASSERT( json_is_string( jsonFunc ) );
      _entryPointGS = json_string_value( jsonFunc );
    }    
  }

  // ps
  {
    const char* func = "func_ps";    
    json_t* jsonFunc = json_object_get( root, func );  
    if( jsonFunc )
    {
      sasASSERT( json_is_string( jsonFunc ) );
      _entryPointPS = json_string_value( jsonFunc );
    }    
  }

  // cs
  {
    const char* func = "func_cs";    
    json_t* jsonFunc = json_object_get( root, func );  
    if( jsonFunc )
    {
      bValid = true;
      sasASSERT( json_is_string( jsonFunc ) );
      _entryPointCS = json_string_value( jsonFunc );
    }    
  }

  sasASSERT(bValid);
}

void sasShaderFamily::loadShaderFlags( json_t* root )
{
  sasShaderFlags* globalFlags = sasShaderMng::singleton().globalShaderFlags();
  json_t* jsonShaderFlags = json_object_get( root, "ShaderFlags" );  
  if( jsonShaderFlags )
  {    
    sasShaderFlags* flags = sasNew sasShaderFlags( jsonShaderFlags );
    _shaderFlags = sasNew sasShaderFlags( globalFlags, flags );
    sasDelete flags;
  }
  else
  {
    _shaderFlags = globalFlags;
  }
}

sasDeviceObject* sasShaderFamily::getShader( sasShaderID id, sasShaderType::Enum type, ShaderMap& shaders )
{
  ShaderMap::const_iterator i =  shaders.find( id );
  if( i != shaders.end() )
    return (*i).second;

  // path
  std::string shaderPath = sasShaderMng::singleton().shaderFolder();
  shaderPath += _fileName;

  // entry point
  const char* entryPoint = 0;
  switch( type )
  {
  case sasShaderType::Vertex: entryPoint =_entryPointVS.c_str(); break;
  case sasShaderType::Hull:   entryPoint =_entryPointHS.c_str(); break;
  case sasShaderType::Domain: entryPoint =_entryPointDS.c_str(); break;
  case sasShaderType::Geometry: entryPoint =_entryPointGS.c_str(); break;
  case sasShaderType::Pixel:  entryPoint =_entryPointPS.c_str(); break;
  case sasShaderType::Compute: entryPoint = _entryPointCS.c_str(); break;
  default: sasASSERT( 0 );
  }  

  // flags
  sasShaderMacro* macros = new sasShaderMacro[ _shaderFlags->size() ];// sasSTACK_ALLOC( sasShaderMacro, _shaderFlags->size() );  
  size_t nMacros = 0;
  if( _shaderFlags->size() )
  {
    nMacros = _shaderFlags->fillMacros( id, macros, _shaderFlags->size() );
  }  

  // build
  sasDeviceObject* d = sasDeviceFactory::singleton().createShader( shaderPath.c_str(), entryPoint, type, &macros[0], nMacros );
  delete [] macros;
  if( d )
    shaders[ id ] = d;    
  return d;
}

void sasShaderFamily::releaseShaders( ShaderMap& shaders )
{
  ShaderMap::iterator i = shaders.begin();
  ShaderMap::iterator last = shaders.end();

  while( i!=last )
  {
    sasDeviceObject* d = (*i).second;
    sasDeviceFactory::singleton().releaseShader( d );
    ++i;
  }
  shaders.clear();
}