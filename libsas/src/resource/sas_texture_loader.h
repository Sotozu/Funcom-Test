#pragma once 

#include "sas_resourceloader.h"

class sasTextureResource;

class sasTextureLoader : public sasResourceLoader
{
  sasMEM_OP( sasTextureLoader );

  sasTextureLoader();
  virtual ~sasTextureLoader();

  virtual sasResource* load( const char* path );

private:
  typedef std::vector< std::string > TexArrayDesc;
  void loadTexArrayDesc( const char* path, TexArrayDesc& texArrayDesc );

  sasTextureResource* loadTexture2d( const char* path );
  sasTextureResource* loadTextureArray2d( const char* path );

};
