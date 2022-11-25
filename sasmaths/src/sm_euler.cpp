#include "sm_euler.h"
#include "sm_matrix.h"

void smEulerAngles::normalize()
{
  while( Yaw >= 360.f )   Yaw -= 360.f;
  while( Yaw < 0.f )      Yaw += 360.f;
  if( Pitch >= 90.f )      Pitch = 89.999f;
  if( Pitch <= -90.f )     Pitch = -89.999f;
  while( Roll >= 360.f )  Roll -= 360.f;
  while( Roll < 0.f )     Roll += 360.f;
}

void smEulerAngles::getRotationMatrix( smMat44& m ) const
{  
  smMat44 mYaw, mPitch;
  // yaw * pitch * roll
  smGetRotJMatrix( Yaw, mYaw);
  smGetRotIMatrix( Pitch, mPitch );
  smGetRotKMatrix( Roll, m );
  smMul( mPitch, m, m );
  smMul( mYaw, m, m );
}