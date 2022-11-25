#pragma once 

#include <string>
#include "utils/sas_stringid.h"

class sasModelResource;
class sasInstance;

struct sasAnimationLayer
{
  sasStringId _animId;
  float _animTime;
  float _blendAmnt;
  sasStringId _boneSetMask;
};

class sasModelInstance
{
  sasNO_COPY( sasModelInstance );
  sasMEM_OP( sasModelInstance );

public:
  sasModelInstance( sasModelResource* resource, sasStringId instanceId );
  ~sasModelInstance();

  void setPosition( const smVec3& position ) { _position = position; _dirty = true; updateInstances(); }
  void setScale( const smVec3& scale ) { _scale = scale; _dirty = true; updateInstances(); }
  void setRotation( const smVec3& rotation ) { _rotation = rotation; _dirty = true; updateInstances(); }
  void setVisible( bool visibe ) { _visible = visibe; _dirty = true; updateInstances(); }
  void addAnimationLayer( const sasStringId& animId, const sasStringId& boneSetMask, float animTime, float blendCoeff );
  void setTintColour( const smVec4& tintCol ) { _tintColour = tintCol; updateInstances(); }

  sasStringId instanceId() const    { return _instanceId; }
  const smVec3& position() const    { return _position; }
  const smVec3& scale() const       { return _scale; }
  const smVec3& rotation() const    { return _rotation; }
  const smVec4& tintColour() const  { return _tintColour; }
  bool visible() const { return _visible; }
  const smMat44& transform();

  bool    skinned() const { return _skinned; }
  size_t  numInstances() const { return _instances.size(); }
  sasInstance* instance( size_t index ) const { sasASSERT( index < _instances.size() ); return _instances[ index ]; }

  void stepPreRender();
  void renderSkeleton() const;

  void setAnimationPaused( bool state ) { _animationPaused = state; }

  void setInstanceData( void* data, uint32_t size );
  void setInstanceCount( uint32_t numInstances );

private:
  void updateInstances();

private:
  std::vector< sasInstance* > _instances;

  sasStringId _instanceId;
  sasModelResource* _modelResource;
  smMat44 _transform;
  smVec3 _position;
  smVec3 _scale;
  smVec3 _rotation;
  smVec4 _tintColour;

  std::vector< sasAnimationLayer > _animLayers;

  bool _visible;
  bool _skinned;
  bool _dirty;
  bool _animationPaused;
};
