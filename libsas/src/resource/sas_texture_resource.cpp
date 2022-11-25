#include "sas_pch.h"
#include "sas_texture_resource.h"
#include "render/sas_texture.h"
#include "sas_resourcemng.h"

sasTextureResource::sasTextureResource( sasTextureId id, sasTexture* texture, const char* path )
    : _texture( texture )
    , _id( id )
{
  if( path != nullptr )
  {
    _path = path;
  }
  //sasTextureMng::singletonPtr()->registerTexture( this );
}

sasTextureResource::~sasTextureResource()
{
  //sasTextureMng::singletonPtr()->unregisterTexture( this );
  sasDelete _texture;
}
