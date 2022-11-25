#pragma once

#include "OVR.h"

class sgOvrInputMng
{
  sasNO_COPY( sgOvrInputMng );
  sasMEM_OP( sgOvrInputMng );

public:
  sgOvrInputMng();
  ~sgOvrInputMng();

  void step();

  bool getOVRInput( float dt, float& yawDelta, float& yaw, float& pitch, float& roll, smVec3& posOffset );

  const std::string&  displayName() const    { return _ovrDisplayName; }
  unsigned int        displayId() const      { return _ovrDisplayId; }
  bool                isVREnabled() const    { return ( _ovrHMD != nullptr ); }
  ovrHmd              ovrHmdPtr() const      { return _ovrHMD; }


private:
  bool initializeOVR();
  void shutdownOVR(); 

private:
  ovrHmd                _ovrHMD;
  std::string           _ovrDisplayName;
  unsigned int          _ovrDisplayId;

};