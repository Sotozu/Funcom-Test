#pragma once 

#include "sas_rendertypes.h"

class sasTextureResource;
struct sasShaderID;
class sasVirtualFile;

struct sasMaterialLayer
{
  enum layerType 
  {
    DiffuseMapLayer = 0,
    NormalMapLayer,
    SpecMapLayer,
    MaskMapLayer,
    HeightMapLayer,
    InvalidLayer
  };
  sasTextureResource* texture;
  layerType           type;

  sasMaterialLayer()
    : texture(0),
    type(InvalidLayer)
  {}

  uint32_t getLayerTextureIndex()
  {
    switch ( type )
    {
    case DiffuseMapLayer:
      return 0;

    case NormalMapLayer:
      return 1;

    case SpecMapLayer:
      return 2;

    case MaskMapLayer:
      return 3;

    case HeightMapLayer:
      return 4;

    default:
      {
         //sasASSERT( false ); 
         return 0;
      }
    }
  }
};

class sasMaterial
{
  sasNO_COPY( sasMaterial );
  sasMEM_OP( sasMaterial );

public:
  sasMaterial( const sasMaterialLayer* layers, size_t nLayers );
  ~sasMaterial();

  size_t layerCount() const { return _nLayers; }
  sasMaterialLayer* layer( size_t i ) const { return &_layers[i]; }

  void                    applyMaterialShaderFlags( sasShaderID& shaderID );
  bool                    isAlphaTested() const     { return _type == sasMaterialType::AlphaTested; }
  bool                    isAlphaBlended() const    { return _type == sasMaterialType::AlphaBlended; }
  sasMaterialType::Enum   materialType() const      { return _type; }

  bool                    serialize( sasVirtualFile& vFile ) const;

private:
  sasMaterialLayer*     _layers;
  size_t                _nLayers;
  sasMaterialType::Enum _type;

  static const unsigned int kVersion = 1;
};