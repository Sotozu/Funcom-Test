#pragma once

#include "utils/sas_stringid.h"
#include "utils/sas_singleton.h"

#include "threading/sas_thread.h"
#include "threading/sas_semaphore.h"
#include "threading/sas_atomic.h"

class sasCamera;
class sgInputMng;
class sgGameObjectMng;
class sgCameraMng;
class seEditor;
class sgPlayfieldMng;
class saAudioMng;
class sgBoidMng;
class ThreadPool;
/*
* Moved _worldMin and _worldMax to from sg_boid.h to here. Set them as static variables that we can access from individual boids.
* Added a ThreadPool class to multithread process in the game.
*/
namespace sg
{
  namespace gamecode
  {
    class Player;
    class World;
  }
}


class sgGameMng : public sasSingleton< sgGameMng >
{
  friend uint32_t sasStdCall gameCodeThreadFn( void* args );

public:
    typedef smVec3 smVec3_6[6];
  sgGameMng();
  ~sgGameMng();

  void initialize( HWND hwnd, const sasConfig& config );
  void destroy();

  void loadPf( sasStringId pfId )                   {}
  void loadScene( sasStringId pfId );

  void step();

  void loadSingleRsc( const std::string& path )     {}

  const char* defaultRscPath() const                { return _rscPath.c_str(); }

  sgInputMng* inputMng() const                      { return _inputMng; }

  void stepGamecodeThread( float dt );

  //NEW - Alex Soto
  //Don't want to edit.
  const smVec3& GetWorldMin() const;
  //NEW - Alex Soto
  //Don't want to edit
  const smVec3& GetWorldMax() const;

  //NEW - Alex Soto
  const smVec3_6& sgGameMng::GetWorldEdgePoints() const;
  const smVec3_6& sgGameMng::GetWorldEdgeNormals() const;

  //NEW - Alex Soto
  //Accessors to private data
  const ThreadPool& GetThreadPool() const;
  ThreadPool& GetThreadPool();
private:
  void initializeSplash();
  void destroySplash();
  void stepCamera( float dt );

private:
  //NEW - Alex Soto
  //These variables are going to be constantly access and making them static and in the stack will be good for this.
    static smVec3 _worldMin;
    static smVec3 _worldMax;
    static smVec3 _worldEdgePoints[6];
    static smVec3 _worldEdgeNormals[6];


  sgCameraMng*          _camMng;
  sgInputMng*           _inputMng;
  sasCamera*            _camera;
  ThreadPool*           _threadPool;

  std::string           _rscPath;

  sasStringId           _loadingSplashScreen;

  struct GameCodeThreadData
  {
    sasSemaphore  _syncWithEngineDone;
    sasSemaphore  _gamecodeDone;
    sasAtomic     _threadAlive;
    sgGameMng*    _gameMng;
  };
  GameCodeThreadData  _threadData;
  sasThread           _gamecodeThread;

  sgBoidMng* _boidMng;



};

//const smVec3 worldEdgePoint[6] = { smVec3(_worldMin.X, 0.f, 0.f),
//                          smVec3(_worldMax.X, 0.f, 0.f),
//                          smVec3(0.f, _worldMin.Y, 0.f),
//                          smVec3(0.f, _worldMax.Y, 0.f),
//                          smVec3(0.f, 0.f, _worldMin.Z),
//                          smVec3(0.f, 0.f, _worldMax.Z) };
//
//const smVec3 worldEdgeNormals[6] = { smVec3(1.f, 0.f, 0.f),
//                            smVec3(-1.f, 0.f, 0.f),
//                            smVec3(0.f, 1.f, 0.f),
//                            smVec3(0.f, -1.f, 0.f),
//                            smVec3(0.f, 0.f, 1.f),
//                            smVec3(0.f, 0.f, -1.f) };


