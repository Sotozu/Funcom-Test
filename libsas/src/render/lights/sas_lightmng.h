#pragma once
#include "utils/sas_singleton.h"

class sasPointLight;
class sasSpotLight;
class sasFrustum;

class sasLightMng : public sasSingleton< sasLightMng >
{
  sasNO_COPY( sasLightMng );
  sasMEM_OP( sasLightMng );

public:
  sasLightMng();
  ~sasLightMng();

  const std::vector< sasPointLight* >&  pointLightArray() const         { return _pointLightArray; }
  const std::vector< sasPointLight* >&  culledPointLightArray() const   { return _culledPointLightArray; }
  const std::vector< sasSpotLight* >&   spotLightArray() const          { return _spotLightArray; }
  const std::vector< sasSpotLight* >&   culledSpotLightArray() const    { return _culledSpotLightArray; }

  //directional light
  void setSunColour( const smVec3& colour ) { _sunColour = colour; }
  void setSunDir( const smVec3& dir )       { _sunDir = dir; smNormalize3( _sunDir, _sunDir ); }
  const smVec3& sunColour() const           { return _sunColour; }
  const smVec3& sunDir() const              { return _sunDir; }

  //point lights
  int createPointLight( sasStringId id, const smVec4& position, float radius, const smVec4& colour, bool enabled );
  void deletePointLight( int lightIndex );
  void deletePointLight( sasStringId id );

  //spot lights
  int createSpotLight( sasStringId id, const smVec4& position, const smVec4& direction, float innerAngle, float outerAngle, float radius, const smVec4& colour, bool enabled );
  void deleteSpotLight( int lightIndex );
  void deleteSpotLight( sasStringId id );

  sasSpotLight* getSpotLight( sasStringId id ) const;
  sasPointLight* getPointLight( sasStringId id ) const;

  //light management
  void cullLights( const sasFrustum& frustum );

  void clearPointLightArray();
  void clearSpotLightArray();

private:
  int getSpotLightIndex( sasStringId id ) const;
  int getPointLightIndex( sasStringId id ) const;

private:
  //local lights
  std::vector< sasPointLight* >   _pointLightArray;
  std::vector< sasPointLight* >   _culledPointLightArray;

  std::vector< sasSpotLight* >    _spotLightArray;
  std::vector< sasSpotLight* >    _culledSpotLightArray;

  //directional light, limit of one at the moment
  smVec3 _sunColour;
  smVec3 _sunDir;

  static const int kPointLightDataVersion = 2;
  static const int kSpotLightDataVersion = 3;
};