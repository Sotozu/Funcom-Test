#include "sas_pch.h"
#include "sas_material.h"
#include "resource/sas_texture_resource.h"
#include "render/shaders/sas_shaderids.h"
#include "utils/sas_virtual_file.h"

sasMaterial::sasMaterial( const sasMaterialLayer* layers, size_t nLayers )
  : _layers(0)
  , _nLayers( nLayers )
{
  _layers = (sasMaterialLayer*)sasMalloc( _nLayers * sizeof(sasMaterialLayer) );
  memcpy( _layers, layers, _nLayers * sizeof(sasMaterialLayer) );

  _type = sasMaterialType::Opaque;
  for( size_t i=0; i<_nLayers; i++ )
  {
    if( _layers[i].type == sasMaterialLayer::MaskMapLayer )
    {
      _type = sasMaterialType::AlphaTested;
    }
  }
}

sasMaterial::~sasMaterial()
{
  for( size_t i=0; i<_nLayers; i++ )
  {
    sasDelete _layers[i].texture;
  }
  sasFree( _layers );
}

void sasMaterial::applyMaterialShaderFlags( sasShaderID& shaderID )
{
  for( size_t i=0; i<_nLayers; i++ )
  {
    if( _layers[i].type == sasMaterialLayer::DiffuseMapLayer )
    {
      shaderID = shaderID | diffuseMask;
    }
    else if( _layers[i].type == sasMaterialLayer::NormalMapLayer )
    {
      shaderID = shaderID | normalMapMask;
    }
    else if( _layers[i].type == sasMaterialLayer::SpecMapLayer )
    {
      shaderID = shaderID | specMapMask;
    }
    else if( _layers[i].type == sasMaterialLayer::MaskMapLayer )
    {
      shaderID = shaderID | maskMapMask;
    }
    else if( _layers[i].type == sasMaterialLayer::HeightMapLayer )
    {
      shaderID = shaderID | heightMapMask;
    }
  }
}

bool sasMaterial::serialize( sasVirtualFile& vFile ) const
{
  sasTextureId invalidId = 0;
  vFile.write( kVersion );
  vFile.write( static_cast< uint32_t >( _nLayers ) );
  vFile.write( _type );
  for( size_t i = 0; i < _nLayers; i++ )
  {
    sasMaterialLayer* l = &( _layers[ i ] );
    vFile.write( l->type );
    vFile.write( l->texture ? l->texture->id() : invalidId );
    if( l->texture )
      vFile.writeStr( l->texture->path() );
  }

  return true;
}
