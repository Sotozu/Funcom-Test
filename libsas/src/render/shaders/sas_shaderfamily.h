#pragma once 

#include "../sas_rendertypes.h"

class sasShaderFlags;
class sasDeviceObject;
class sasStringId;

class sasShaderFamily
{
  sasNO_COPY(sasShaderFamily);
  sasMEM_OP(sasShaderFamily);

public:
  sasShaderFamily( json_t* root );
  ~sasShaderFamily();
    
public:
  sasShaderID getOptionBitMask( const sasStringId& opt, size_t value = 1 );
  sasDeviceObject* getShader( sasShaderID id, sasShaderType::Enum type );    
  void releaseShaders();

private:
  typedef std::map< sasShaderID, sasDeviceObject* > ShaderMap;

private:
  void loadFileName( json_t* root );
  void loadEntryPoints( json_t* root );
  void loadShaderFlags( json_t* root );
  sasDeviceObject* getShader( sasShaderID id, sasShaderType::Enum type, ShaderMap& shaders );
  void releaseShaders( ShaderMap& shaders );

private:
  std::string     _fileName;
  std::string     _entryPointVS;
  std::string     _entryPointHS;
  std::string     _entryPointDS;
  std::string     _entryPointGS;
  std::string     _entryPointPS;
  std::string     _entryPointCS;
  sasShaderFlags* _shaderFlags;
  ShaderMap       _vertexShaders;
  ShaderMap       _hullShaders;
  ShaderMap       _domainShaders;
  ShaderMap       _geometryShaders;
  ShaderMap       _pixelShaders;  
  ShaderMap       _computeShaders;
};