#pragma once 

#include "utils/sas_stringid.h"
#include "Types.h"

class sasVirtualFile;

struct sasPositionFrame
{
  smVec3 _position;
  double _time;
};

struct sasRotationFrame
{
  aiQuaternion _rotation;
  double  _time;
};

struct sasScalingFrame
{
  smVec3 _scale;
  double _time;
};

class sasAnimationChannel
{
  sasMEM_OP( sasAnimationChannel );

  static const int kMaxFrames = 512;

public:
  smVec3        computeInterpolatedScaling( float animationTime ) const;
  smVec3        computeInterpolatedTranslation( float animationTime ) const;
  aiQuaternion  computeInterpolatedRotation( float animationTime ) const;

  bool serialize( sasVirtualFile& vFile ) const;

private:
  unsigned int findLastScalingFrameIndex( float animationTime ) const;
  unsigned int findLastPositionFrameIndex( float animationTime ) const;
  unsigned int findLastRotationFrameIndex( float animationTime ) const;

public:
  sasStringId   _boneId;

	unsigned int      _numPositionFrames;
  unsigned int      _numRotationFrames;
  unsigned int      _numScalingFrames;
  sasPositionFrame  _positionFrames[ kMaxFrames ];
  sasRotationFrame  _rotationFrames[ kMaxFrames ];
  sasScalingFrame   _scalingFrames[ kMaxFrames ];
};


class sasAnimation
{
  sasMEM_OP( sasAnimation );

public:
  sasAnimation( sasStringId name, double ticks, double ticksPerSecond, unsigned int numChannels );
  ~sasAnimation();

  const sasAnimationChannel* findChannel( sasStringId channelId ) const;

  bool serialize( sasVirtualFile& vFile ) const;

public:
  sasStringId   _animationId;
	double        _animationTicksDuration;
	double        _ticksPerSecond;
	unsigned int  _numChannels;

	sasAnimationChannel* _channels;

  static const unsigned int kVersion = 1;

};


