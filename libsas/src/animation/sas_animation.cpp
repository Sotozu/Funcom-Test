#include "sas_pch.h"
#include "sas_animation.h"
#include "utils/sas_virtual_file.h"

sasAnimation::sasAnimation( sasStringId name, double ticks, double ticksPerSecond, unsigned int numChannels )
  : _numChannels( numChannels )
  , _animationId( name )
  , _animationTicksDuration( ticks )
  , _ticksPerSecond( ticksPerSecond )
{
  _channels = sasNew sasAnimationChannel[ numChannels ];
}

sasAnimation::~sasAnimation()
{
  sasDelete [] _channels;
}

const sasAnimationChannel* sasAnimation::findChannel( sasStringId channelId ) const
{
  for( unsigned int i = 0; i < _numChannels; i++ )
  {
    if( _channels[ i ]._boneId == channelId )
    {
      return &_channels[ i ];
    }
  }
  return NULL;
}

smVec3 sasAnimationChannel::computeInterpolatedScaling( float animationTime ) const
{
  if ( _numScalingFrames == 1 ) 
  {
    return _scalingFrames[ 0 ]._scale;
  }

  unsigned int index = findLastScalingFrameIndex( animationTime );
  unsigned int nextIndex = index + 1;
  sasASSERT( nextIndex < _numPositionFrames );
  float deltaTime = ( float )( _scalingFrames[ nextIndex ]._time - _scalingFrames[ index ]._time );
  float blendCoeff = ( animationTime - (float)_scalingFrames[ index ]._time ) / deltaTime;
  //sasASSERT( blendCoeff >= 0.f && blendCoeff <= 1.f );
  blendCoeff = std::max< float >( 0.f, blendCoeff );
  const smVec3& startScale = _scalingFrames[ index ]._scale;
  const smVec3& endScale = _scalingFrames[ nextIndex ]._scale;
  smVec3 res = smLerp( startScale, endScale, blendCoeff );
  return res;
}

smVec3 sasAnimationChannel::computeInterpolatedTranslation( float animationTime ) const
{
  if ( _numPositionFrames == 1 ) 
  {
    return _positionFrames[ 0 ]._position;
  }

  unsigned int index = findLastPositionFrameIndex( animationTime );
  unsigned int nextIndex = index + 1;
  sasASSERT( nextIndex < _numPositionFrames );
  float deltaTime = ( float )( _positionFrames[ nextIndex ]._time - _positionFrames[ index ]._time );
  float blendCoeff = ( animationTime - (float)_positionFrames[ index ]._time ) / deltaTime;
  //sasASSERT( blendCoeff >= 0.f && blendCoeff <= 1.f );
  blendCoeff = std::max< float >( 0.f, blendCoeff );
  const smVec3& startPos = _positionFrames[ index ]._position;
  const smVec3& endPos = _positionFrames[ nextIndex ]._position;
  smVec3 res = smLerp( startPos, endPos, blendCoeff );
  return res;
}

aiQuaternion sasAnimationChannel::computeInterpolatedRotation( float animationTime ) const
{  
  if ( _numRotationFrames == 1 ) 
  {
    return _rotationFrames[ 0 ]._rotation;
  }

  unsigned int rotationIndex = findLastRotationFrameIndex( animationTime );
  unsigned int nextRotationIndex = rotationIndex + 1;
  sasASSERT( nextRotationIndex < _numRotationFrames );
  float deltaTime = ( float )( _rotationFrames[ nextRotationIndex ]._time - _rotationFrames[ rotationIndex ]._time );
  float blendCoeff = ( animationTime - (float)_rotationFrames[ rotationIndex ]._time ) / deltaTime;
  //sasASSERT( blendCoeff >= 0.f && blendCoeff <= 1.f );
  blendCoeff = std::max< float >( 0.f, blendCoeff );
  const aiQuaternion& startRotationQ = _rotationFrames[ rotationIndex ]._rotation;
  const aiQuaternion& endRotationQ = _rotationFrames[ nextRotationIndex ]._rotation;
  aiQuaternion res;
  aiQuaternion::Interpolate( res, startRotationQ, endRotationQ, blendCoeff );
  res = res.Normalize();
  return res;
}

unsigned int sasAnimationChannel::findLastScalingFrameIndex( float animationTime ) const
{
  for( unsigned int i = 0; i < _numScalingFrames - 1; i++ )
  {
    if( animationTime < _scalingFrames[ i + 1 ]._time )
    {
      return i;
    }
  }
  sasASSERT( false );
  return 0;
}

unsigned int sasAnimationChannel::findLastPositionFrameIndex( float animationTime ) const
{
  for( unsigned int i = 0; i < _numPositionFrames - 1; i++ )
  {
    if( animationTime < _positionFrames[ i + 1 ]._time )
    {
      return i;
    }
  }
  sasASSERT( false );
  return 0;
}

unsigned int sasAnimationChannel::findLastRotationFrameIndex( float animationTime ) const
{
  for( unsigned int i = 0; i < _numRotationFrames - 1; i++ )
  {
    if( animationTime < _rotationFrames[ i + 1 ]._time )
    {
      return i;
    }
  }
  sasASSERT( false );
  return 0;
}

bool sasAnimation::serialize( sasVirtualFile& vFile ) const
{
  vFile.write( kVersion );

  const char* name = _animationId.string();
  vFile.writeStr( name );

  vFile.write( _animationTicksDuration );
  vFile.write( _ticksPerSecond );
  vFile.write( _numChannels );

  for( uint32_t i = 0; i < _numChannels; i++ )
  {
    _channels[ i ].serialize( vFile );
  }

  return true;
}

bool sasAnimationChannel::serialize( sasVirtualFile& vFile ) const
{
  const char* boneName = _boneId.string();
  vFile.writeStr( boneName );

  vFile.write( _numPositionFrames );
  for( uint32_t i = 0; i < _numPositionFrames; i++ )
  {
    vFile.write( _positionFrames[ i ]._time );
    vFile.write( _positionFrames[ i ]._position.X );
    vFile.write( _positionFrames[ i ]._position.Y );
    vFile.write( _positionFrames[ i ]._position.Z );
  }

  vFile.write( _numRotationFrames );
  for( uint32_t i = 0; i < _numRotationFrames; i++ )
  {
    vFile.write( _rotationFrames[ i ]._time );
    vFile.write( _rotationFrames[ i ]._rotation.x );
    vFile.write( _rotationFrames[ i ]._rotation.y );
    vFile.write( _rotationFrames[ i ]._rotation.z );
    vFile.write( _rotationFrames[ i ]._rotation.w );
  }

  vFile.write( _numScalingFrames );
  for( uint32_t i = 0; i < _numScalingFrames; i++ )
  {
    vFile.write( _scalingFrames[ i ]._time );
    vFile.write( _scalingFrames[ i ]._scale.X );
    vFile.write( _scalingFrames[ i ]._scale.Y );
    vFile.write( _scalingFrames[ i ]._scale.Z );
  }

  return true;
}

