#include "sas_pch.h"
#include "sas_lightmng.h"
#include "sas_light.h"
#include "utils/sas_file.h"
#include "resource/sas_resourcemng.h"
#include "utils/sas_frustum.h"
#include "utils/sas_culling.h"
#include "render/debug/sas_debugmng.h"

#include "utils/sas_reflection.h"

sasLightMng::sasLightMng( )
{
  _pointLightArray.reserve( 3000 );
  _culledPointLightArray.reserve( 3000 );
  _spotLightArray.reserve( 1000 );
  _culledSpotLightArray.reserve( 1000 );

  //sunlight
  setSunDir( smVec3( 0.5f, -0.8f, 0.15f ) );
  setSunColour( smVec3( 0.28f, 0.3f, 0.5f ) );
}

sasLightMng::~sasLightMng( )
{
  clearPointLightArray();
  clearSpotLightArray();
}

int sasLightMng::createPointLight( sasStringId id, const smVec4& position, float radius, const smVec4& colour, bool enabled )
{
  sasPointLight* light = sasNew sasPointLight( id, position, colour, radius, enabled ); 
  _pointLightArray.push_back( light );
  return _pointLightArray.size() - 1;
}

void sasLightMng::deletePointLight( int lightIndex )
{
  sasASSERT( lightIndex >= 0 && lightIndex < static_cast< int >( _pointLightArray.size() ) );
  sasDelete _pointLightArray[ lightIndex ];
  _pointLightArray[ lightIndex ] = _pointLightArray[ _pointLightArray.size() - 1 ];
  _pointLightArray.pop_back();
}

void sasLightMng::deletePointLight( sasStringId lightId )
{
  int index = getPointLightIndex( lightId );
  if( index >= 0 )
  {
    deletePointLight( index );
  }
}

int sasLightMng::createSpotLight( sasStringId id, const smVec4& position, const smVec4& direction, float innerAngle, float outerAngle, float radius, const smVec4& colour, bool enabled )
{
  sasSpotLight* light = sasNew sasSpotLight( id, position, colour, direction, innerAngle, outerAngle, radius, enabled, false );
  _spotLightArray.push_back( light );
  return _spotLightArray.size() - 1;
}

void sasLightMng::deleteSpotLight( int lightIndex )
{
  sasASSERT( lightIndex >= 0 && lightIndex < static_cast< int >( _spotLightArray.size() ) );
  sasDelete _spotLightArray[ lightIndex ];
  _spotLightArray[ lightIndex ] = _spotLightArray[ _spotLightArray.size() - 1 ];
  _spotLightArray.pop_back();
}

void sasLightMng::deleteSpotLight( sasStringId lightId )
{
  int index = getSpotLightIndex( lightId );
  if( index >= 0 )
  {
    deleteSpotLight( index );
  }
}

int sasLightMng::getSpotLightIndex( sasStringId id ) const
{
  for( size_t i = 0; i < _spotLightArray.size(); i++ )
  {
    if( _spotLightArray[ i ]->id() == id )
      return i;
  }

  return -1;
}

int sasLightMng::getPointLightIndex( sasStringId id ) const
{
  for( size_t i = 0; i < _pointLightArray.size(); i++ )
  {
    if( _pointLightArray[ i ]->id() == id )
      return i;
  }

  return -1;
}

sasSpotLight* sasLightMng::getSpotLight( sasStringId id ) const
{
  int index = getSpotLightIndex( id );
  if( index >= 0 )
    return _spotLightArray[ index ];

  return NULL;
}

sasPointLight* sasLightMng::getPointLight( sasStringId id ) const
{
  int index = getPointLightIndex( id );
  if( index >= 0 )
    return _pointLightArray[ index ];

  return NULL;
}

void sasLightMng::clearPointLightArray()
{
  std::vector<sasPointLight*>::iterator iLight = _pointLightArray.begin();
  std::vector<sasPointLight*>::iterator iLightEnd = _pointLightArray.end();
  for( ; iLight != iLightEnd; ++iLight )
  {
    delete *iLight;
  }
  _pointLightArray.clear();
}

void sasLightMng::clearSpotLightArray()
{
  std::vector<sasSpotLight*>::iterator iLight = _spotLightArray.begin();
  std::vector<sasSpotLight*>::iterator iLightEnd = _spotLightArray.end();
  for( ; iLight != iLightEnd; ++iLight )
  {
    delete *iLight;
  }
  _spotLightArray.clear();
}

void sasLightMng::cullLights( const sasFrustum& frustum )
{
  //point lights
  _culledPointLightArray.clear();

  if( sasDebugMng::singletonPtr()->lightCullingEnabled() )
  {
    for( size_t i = 0; i < _pointLightArray.size(); i++ )
    {
      smVec4 lightVolume( _pointLightArray[ i ]->position() );
      lightVolume.W = _pointLightArray[ i ]->radius();
      if( sasIntersects( &frustum, &lightVolume ) )
      {
        _culledPointLightArray.push_back( _pointLightArray[ i ] );
      }
    }
  }
  else
  {
    for( size_t i = 0; i < _pointLightArray.size(); i++ )
    {
       _culledPointLightArray.push_back( _pointLightArray[ i ] );
    }
  }

  
  //spot lights
  _culledSpotLightArray.clear();

  if( sasDebugMng::singletonPtr()->lightCullingEnabled() )
  {
    for( size_t i = 0; i < _spotLightArray.size(); i++ )
    {
      smVec4 lightVolume( _spotLightArray[ i ]->position() );
      lightVolume.W = _spotLightArray[ i ]->radius();
      if( sasIntersects( &frustum, &lightVolume ) )
      {
        _culledSpotLightArray.push_back( _spotLightArray[ i ] );
      }
    }
  }
  else
  {
    for( size_t i = 0; i < _spotLightArray.size(); i++ )
    {
      _culledSpotLightArray.push_back( _spotLightArray[ i ] );
    }
  }
  
}
