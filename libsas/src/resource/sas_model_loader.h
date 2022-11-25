#pragma once 

#include "sas_resourceloader.h"


class sasModelLoader : public sasResourceLoader
{
  sasMEM_OP( sasModelLoader );

  virtual sasResource* load( const char* path );
  virtual sasResource* load( const char* path, const char* name );
  virtual sasResource* load( const char* path, const char* name, const smVec3& scale, const smVec3& translation );
};

sasResource* createBlobModelResource( sasStringId id );
