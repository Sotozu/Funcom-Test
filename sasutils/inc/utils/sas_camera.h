#pragma once 

#include "sasMaths.h"

class sasCamera
{
  //sasNO_COPY( sasCamera );
  sasMEM_OP( sasCamera );

public:
  sasCamera();
  ~sasCamera();

public:  
  
  const smVec4&     position() const                  { return _position; }
  void              setPosition( const smVec4& p )    { _position.set( p.X, p.Y, p.Z, 1.f ); }
  void              setFront( const smVec4& f )       { _front.set( f.X, f.Y, f.Z, 0.f ); }
  void              setUp( const smVec4& u )          { _up.set( u.X, u.Y, u.Z, 0.f ); }
  const smVec4&     front() const                     { return _front; }
  const smVec4&     up() const                        { return _up; }
  float             zNear() const                     { return _zNear; }  
  float             zFar() const                      { return _zFar; }  
  void              setZPlanes( float znear, float zfar );    
  float             FOV() const                       { return _fovX; }
  void              setFOV( float a );
  
  void              lookAt( const smVec4& target, const smVec4& up );
  void              getRight( smVec4& r ) const;
  void              getViewMatrix( smMat44& m ) const;
  void              getProjMatrix( float screenWidth, float screenHeight, smMat44& m ) const;
  void              getProjMatrix_Ortho( float width, float height, smMat44& m ) const;

private:
  smVec4  _position;
  smVec4  _front;
  smVec4  _up;  
  float   _fovX;
  float   _zNear;
  float   _zFar;
};

