#pragma once

#include "utils/sas_stringid.h"

class sasPointLight
{
  sasNO_COPY( sasPointLight );
  sasMEM_OP( sasPointLight );
public:
  sasPointLight( sasStringId id, const smVec4& position, const smVec4& colour, float radius, bool enabled );
  ~sasPointLight();

  const smVec4&  position() const { return _position; }
  const smVec4&  colour() const   { return _colour; }
  float   radius() const          { return _radius; }
  bool    enabled() const         { return _enabled; }
  sasStringId id() const          { return _id; }

  void  setPosition( const smVec4& position ) { _position = position; }
  void  setColour( const smVec4& colour )     { _colour = colour; }
  void  setRadius( float radius )       { _radius = radius; }
  void  setEnabled( bool enabled )      { _enabled = enabled; }

private:
  sasStringId _id;
  smVec4  _position;
  smVec4  _colour;
  float   _radius;
  bool    _enabled;

};

class sasSpotLight
{
  sasNO_COPY( sasSpotLight );
  sasMEM_OP( sasSpotLight );
public:
  sasSpotLight( sasStringId id, const smVec4& position, const smVec4& colour, const smVec4& direction, float innerAngle, float outerAngle, float radius, bool enabled, bool castsShadows );
  ~sasSpotLight();

  const smVec4&  position() const   { return _position; }
  const smVec4&  colour() const     { return _colour; }
  const smVec4&  direction() const  { return _direction; }
  float   radius() const            { return _radius; }
  float   innerAngle() const        { return _innerAngle; }
  float   outerAngle() const        { return _outerAngle; }
  bool    enabled() const           { return _enabled; }
  int     shadowMapIndex() const    { return _shadowmapIndex; }
  bool    castsShadows() const      { return _castsShadows; }
  sasStringId id() const            { return _id; }

  void  setPosition( const smVec4& position )   { _position = position; }
  void  setDirection( const smVec4& direction ) { _direction = direction; }
  void  setColour( const smVec4& colour )       { _colour = colour; }
  void  setInnerAngle( float angle )            { _innerAngle = angle; }
  void  setOuterAngle( float angle )            { _outerAngle = angle; }
  void  setRadius( float radius )               { _radius = radius; }
  void  setEnabled( bool enabled )              { _enabled = enabled; }
  void  setShadowMapIndex( int index )          { _shadowmapIndex = index; }
  void  setCastsShadows( bool state );

private:
  sasStringId _id;
  smVec4  _position;
  smVec4  _colour;
  smVec4  _direction;
  float   _radius;
  float   _innerAngle;
  float   _outerAngle;
  uint32_t _shadowmapIndex;
  bool    _enabled;
  bool    _castsShadows;
};


