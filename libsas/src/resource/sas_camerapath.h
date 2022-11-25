#pragma once 

#include "sm_common.h"
#include <string>
#include "utils/sas_stringid.h"

class sasCamera;

smALIGN16_PRE class sasCameraPathNode
{
public:
  smVec4 _position;
  smVec4 _front;
  smVec4 _up;
  float _timeOffset;
} smALIGN16_POST;

class sasCameraPath
{
  sasNO_COPY( sasCameraPath );
  sasMEM_OP( sasCameraPath );

public:
  sasCameraPath( sasStringId cameraPathId, const char* strName );
  ~sasCameraPath();

  void          setName( const char* name );
  const char*   name() const { return _strName.c_str(); }
  sasStringId   id() const { return _id; }
  void          updateCamera( sasCamera* camera );
  void          startCameraPath( bool looping );
  void          stopCameraPath();
  void          setCameraPathSpeed( float speed )   { _speed = speed; }
  unsigned int  numNodes() const                    { return _numNodes; }
  void          setNumNodes( unsigned int numNodes ){ _numNodes = numNodes; }
  sasCameraPathNode* pathNodes()                    { return _pathNodes; }
  bool          isDone() const;

  //debug render of camera path nodes
  void          render();

private:
  static const int kMaxNumNodes = 256;

  std::string _strName;
  sasStringId _id;
  sasCameraPathNode _pathNodes[ kMaxNumNodes ];
  unsigned int _numNodes;
  float _startAnimationTime;
  float _speed;
  bool _running;
  bool _looping;

};
