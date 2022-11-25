#pragma once 

#include "utils/sas_singleton.h"

class sasCamera;
class sasCameraPath;

class sasCameraMng : public sasSingleton<sasCameraMng>
{
  sasMEM_OP( sasCameraMng );

public:
  sasCameraMng();
  ~sasCameraMng();

public:
  void addCamera( sasCamera* camera );
  void removeCamera( sasCamera* camera );

  sasCamera* getCurrentCamera();
  void step();
  void runCameraPath( sasCameraPath* cameraPath, bool looping, float speed = 1.f );
  void stopCameraPath();

private:
  int getCameraIndex( sasCamera* camera );

private:
  typedef std::vector< sasCamera* > Cameras;

  Cameras         _cameras;
  int             _current;
  sasCameraPath*  _runningCameraPath;
};
