#include "sas_pch.h"
#include "sas_light.h"
#include "render/shadows/sas_spotshadowpool.h"


sasPointLight::sasPointLight( sasStringId id, const smVec4& position, const smVec4& colour, float radius, bool enabled )
  : _id( id ), _position(position), _colour(colour), _radius(radius), _enabled(enabled)
{
}

sasPointLight::~sasPointLight()
{
}

sasSpotLight::sasSpotLight( sasStringId id, const smVec4& position, const smVec4& colour, const smVec4& direction, float innerAngle, float outerAngle, float radius, bool enabled, bool castsShadows )
  : _id( id ), _position(position), _colour(colour), _direction( direction ), _innerAngle( innerAngle ), _outerAngle( outerAngle ), _radius( radius ), _enabled( enabled ), _castsShadows( castsShadows )
{
  _shadowmapIndex = -1;
  if( _castsShadows )
  {
    sasSpotShadowPool::singletonPtr()->registerSpotLight( this );
  }
}

sasSpotLight::~sasSpotLight()
{
  if( _castsShadows )
  {
    sasSpotShadowPool::singletonPtr()->unregisterSpotLight( this );
  }
}

void sasSpotLight::setCastsShadows( bool state )
{
  if( state != _castsShadows )
  {
    _castsShadows = state;
    if( state )
    {
      sasSpotShadowPool::singletonPtr()->registerSpotLight( this );
    }
    else
    {
      sasSpotShadowPool::singletonPtr()->unregisterSpotLight( this );
    }
  }
}
