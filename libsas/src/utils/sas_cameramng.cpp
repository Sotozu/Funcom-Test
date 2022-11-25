#include "sas_pch.h"
#include "sas_cameramng.h"
#include "resource/sas_camerapath.h"

sasCameraMng::sasCameraMng()
  : _current( -1 )
  , _runningCameraPath( NULL )
{
}

sasCameraMng::~sasCameraMng()
{
  sasASSERT( _cameras.empty() );
}

void sasCameraMng::addCamera( sasCamera* camera )
{
  sasASSERT( getCameraIndex( camera ) == -1 );
  _cameras.push_back( camera );
  if( _cameras.size() == 1 )
    _current = 0;
}

void sasCameraMng::removeCamera( sasCamera* camera )
{
  int index = getCameraIndex( camera );
  sasASSERT( index != -1 );    
  _cameras.erase( std::find( _cameras.begin(), _cameras.end(), camera ) );

  if( _cameras.empty() )
  {
    _current = -1;
  }
  else if( _current >=  (int)_cameras.size() )
  {    
    --_current;
  }
}

sasCamera* sasCameraMng::getCurrentCamera()
{
  if( _current == -1 )
    return 0;  
  return _cameras[ _current ];
}

int sasCameraMng::getCameraIndex( sasCamera* camera )
{
  int index = 0;
  Cameras::const_iterator i = _cameras.begin();
  Cameras::const_iterator last = _cameras.end();
  while( i!=last )
  {
    if( *i == camera )
      break;
    ++index;
    ++i;
  }  
  return index==_cameras.size() ? -1 : index;
}

void sasCameraMng::step()
{
  if( _runningCameraPath != NULL )
  {
    sasCamera* cam = getCurrentCamera();
    _runningCameraPath->updateCamera( cam );

    if( _runningCameraPath->isDone() )
    {
      _runningCameraPath = NULL;
    }
  }
}

void sasCameraMng::runCameraPath( sasCameraPath* cameraPath, bool looping, float speed )
{
  sasASSERT( cameraPath != NULL );
  _runningCameraPath = cameraPath;
  _runningCameraPath->setCameraPathSpeed( speed );
  _runningCameraPath->startCameraPath( looping );
}

void sasCameraMng::stopCameraPath()
{
  sasASSERT( _runningCameraPath != NULL );
  _runningCameraPath->stopCameraPath();
  _runningCameraPath = NULL;
}