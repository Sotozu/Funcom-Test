#pragma once 

#include "sm_common.h"

struct smMat44;

smALIGN16_PRE struct smEulerAngles
{
  //! ctor/dtor
  smEulerAngles() 
    : Yaw(0), Pitch(0), Roll(0)
  {
    smALIGNED(this);
  }

  //! methods
  void normalize();
  void getRotationMatrix( smMat44& m ) const;
      
  //! members
  float Yaw;
  float Pitch;
  float Roll;
  float _Pad_;
} smALIGN16_POST;

