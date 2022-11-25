#pragma once 

#include "sas_resource.h"

class sasTexture;

class sasTextureResource : public sasResource 
{
  sasNO_COPY( sasTextureResource );
  sasMEM_OP( sasTextureResource );

public:
    sasTextureResource( sasTextureId id, sasTexture* texture, const char* path );
    ~sasTextureResource();

public:
  sasTexture*   texture() const { return _texture; }
  sasTextureId  id() const      { return _id; }
  const char*   path() const    { return _path.c_str(); }

private:
  sasTexture*   _texture;
  sasTextureId  _id;
  std::string   _path;
};