#pragma once 

#include "utils/sas_singleton.h"

class sasCamera;
class sgCameraPath;
class sgInputMng;

namespace sg
{
  namespace gamecode
  {
    class Player;
  }
}

class sgCameraMng : public sasSingleton<sgCameraMng>
{
  sasNO_COPY( sgCameraMng );
  sasMEM_OP( sgCameraMng );

public:
  sgCameraMng();
  ~sgCameraMng();

public:
  void addCamera( sasCamera* camera );
  void removeCamera( sasCamera* camera );

  sasCamera* getCurrentCamera();
  void step( sgInputMng* inputMng, float deltaTime );
  void runCameraPath( sgCameraPath* cameraPath, bool looping, float speed = 1.f );
  void stopCameraPath();

  void attachCamera( sasCamera* camera, sg::gamecode::Player* target );
  bool isCameraBound( sasCamera* camera ) const;

private:
  int getCameraIndex( sasCamera* camera );

private:
  typedef std::vector< sasCamera* > Cameras;

  Cameras         _cameras;
  int             _current;
  sgCameraPath*  _runningCameraPath;

  std::map< sasCamera*, sg::gamecode::Player* > _cameraBindings;
};
