#include "sas_pch.h"
#include "sas_camerapath.h"
#include "sas_resourcemng.h"
#include "utils/sas_camera.h"
#include "utils/sas_timer.h"
#include "utils/sas_color.h"
#include "render/debug/sas_debugrender.h"
#include "render/sas_rendermng.h"


sasCameraPath::sasCameraPath( sasStringId id, const char* strName )
  : _id( id )
  , _numNodes( 0 )
  , _speed( 1.f )
  , _running( false )
  , _looping( false )
  , _startAnimationTime( 0.f )
{
  _strName = strName;
  sasASSERT( sasStringId::build( strName ) == id );
}

sasCameraPath::~sasCameraPath()
{
}

void sasCameraPath::setName( const char* name )
{
  sasASSERT( name );
  _strName = name;
  _id = sasStringId::build( name );
}

void sasCameraPath::updateCamera( sasCamera* camera )
{
  sasASSERT( camera );
  
  if( _numNodes == 0 )
    return;

  float time = sasTimer::singletonPtr()->getTime();
  float currentTimeOffset = time - _startAnimationTime;
  currentTimeOffset *= _speed;

  if( currentTimeOffset > _pathNodes[ _numNodes - 1 ]._timeOffset )
  {
    if( _looping )
    {
      _startAnimationTime = time;
    }
    else
    {
      stopCameraPath();
      return;
    }
  }

  int indexLow = 0;
  unsigned int indexHigh = 0;
  for( unsigned int i = 0; i < _numNodes; i++ )
  {
    if( _pathNodes[ i ]._timeOffset > currentTimeOffset )
    {
      indexHigh = i;
      indexLow = i - 1;
      break;
    }
  }

  if( indexLow < 0 )
  {
    if( _looping )
      indexLow = _numNodes - 1;
    else
      indexLow = 0;
  }

  float currentStepTimeOffset = currentTimeOffset - _pathNodes[ indexLow ]._timeOffset;
  float stepDuration = _pathNodes[ indexHigh ]._timeOffset - _pathNodes[ indexLow ]._timeOffset;
  float coeff = currentStepTimeOffset / stepDuration;

  sasASSERT( coeff >= 0.f && coeff <= 1.f );

  smVec4 pos = smLerp( _pathNodes[ indexLow ]._position, _pathNodes[ indexHigh ]._position, coeff );
  smVec4 front = smLerp( _pathNodes[ indexLow ]._front, _pathNodes[ indexHigh ]._front, coeff );
  smVec4 up = smLerp( _pathNodes[ indexLow ]._up, _pathNodes[ indexHigh ]._up, coeff );

  smNormalize3( front, front );
  smNormalize3( up, up );

  //make sure up and front are orthonormal
  smVec4 right;
  smCross( front, up, right );
  smNormalize3( right, right );
  smCross( right, front, up );  
  smNormalize3( up, up );

  //update camera
  camera->setPosition( pos );
  camera->setFront( front );
  camera->setUp( up );
}

void sasCameraPath::startCameraPath( bool looping )
{
  sasASSERT( !_running );
  _running = true;
  _startAnimationTime = sasTimer::singletonPtr()->getTime();
  _looping = looping;
  sasRenderMng::singletonPtr()->setUIEnabled( false );
}

void sasCameraPath::stopCameraPath()
{
  sasASSERT( _running );
  _running = false;
  sasRenderMng::singletonPtr()->setUIEnabled( true );
}

void sasCameraPath::render()
{
  if( _numNodes == 0 || _running )
    return;

  sasDebugRender* debugRender = sasDebugRender::singletonPtr();

  float endPathTime = _pathNodes[ _numNodes - 1 ]._timeOffset;

  for( unsigned int i = 0; i < _numNodes; i++ )
  {
    float currentTimeOffset = _pathNodes[ i ]._timeOffset;
    float coeff = currentTimeOffset / ( endPathTime + 1.f );

    sasASSERT( coeff >= 0.f && coeff <= 1.f );

    smVec4 colour = smLerp( smVec4( 0.f, 1.f, 0.f, 1.f ), smVec4( 1.f, 0.f, 0.f, 1.f ), coeff );
    sasColor lineCol( colour.X, colour.Y, colour.Z, colour.W );

    if( ( i + 1 ) < _numNodes )
    {
      debugRender->renderLine( _pathNodes[ i ]._position, _pathNodes[ i + 1 ]._position, lineCol );
    }
    else if( _looping )
    {
      debugRender->renderLine( _pathNodes[ i ]._position, _pathNodes[ 0 ]._position, lineCol );
    }

    debugRender->renderSphere( _pathNodes[ i ]._position, 0.5f, colour );
  }
}

bool sasCameraPath::isDone() const
{
  return !_running;
}