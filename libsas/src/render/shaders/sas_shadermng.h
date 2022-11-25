#pragma once 

#include "utils/sas_singleton.h"
#include "utils/sas_stringid.h"
#include "../sas_rendertypes.h"

class sasDeviceObject;
class sasShaderFlags;
class sasShaderFamily;

class sasShaderMng : public sasSingleton< sasShaderMng >
{
  sasNO_COPY( sasShaderMng );
  sasMEM_OP( sasShaderMng );

public:
  struct Family 
  {
    sasStringId       key;
    sasShaderFamily*  family;
  };

public:
  sasShaderMng( const char* shaderFolder );
  ~sasShaderMng();

public:
  sasShaderFlags*   globalShaderFlags() const { return _globalFlags; }
  const std::string& shaderFolder() const { return _shaderFolder; }

  sasShaderID       getFamilyBitFlag( const sasStringId& key );
  sasShaderID       getOptionBitMask( sasShaderID id, const sasStringId& opt, size_t value = 1 );
  sasShaderID       getGlobalOptionBitMask( const sasStringId& opt, size_t value = 1 );

  sasDeviceObject*  getVertexShader( sasShaderID id );
  sasDeviceObject*  getHullShader( sasShaderID id );
  sasDeviceObject*  getDomainShader( sasShaderID id );
  sasDeviceObject*  getGeometryShader( sasShaderID id );
  sasDeviceObject*  getPixelShader( sasShaderID id );
  sasDeviceObject*  getComputeShader( sasShaderID id );

  void releaseShaders(); 

  void initializeShaderIds();

private:
  void loadSetup();
  void loadGlobalFlags( json_t* root );
  void loadFamilies( json_t* root );
  sasShaderFamily* loadFamily( const char* familyName );
  
  sasShaderFamily* getFamily( const sasStringId& id ) const;
  sasDeviceObject* getShader( sasShaderID id, sasShaderType::Enum type );   

private:
  std::string         _shaderFolder;
  sasShaderFlags*     _globalFlags;
  Family*             _families; 
  size_t              _nFamilies;
};
