#include "su_pch.h"
#include "utils/sas_camera.h"
//#include "sas_cameramng.h"
#include "sasMaths.h"

sasCamera::sasCamera()
  : _position(0,0,0,1)
  , _front( 0,0,1,0 )
  , _up( 0,1,0,0 )
  , _fovX( 90.f )
  , _zNear( 0.5f )
  , _zFar( 100.f )
{  
  //sasCameraMng::singleton().addCamera( this );  
}

sasCamera::~sasCamera()
{
  //sasCameraMng::singleton().removeCamera( this );
}

void sasCamera::setZPlanes( float znear, float zfar )
{
  sasASSERT( znear < zfar );
  _zNear = znear;
  _zFar = zfar;
}

void sasCamera::setFOV( float a )
{  
  _fovX = smClamp( a, 1.f, 179.f );
}

void sasCamera::lookAt( const smVec4& target, const smVec4& up )
{
  if( target == _position )
    return;  
  // compute front vector
  smSub3( target, _position, _front );
  smNormalize3( _front, _front );  
  // compute right
  smVec4 upNm;
  smVec4 right;
  smNormalize3( up, upNm );
  smCross( _front, upNm, right );
  smNormalize3( right, right );
  // compute up
  smCross( right, _front, _up );  
  smNormalize3( _up, _up );
}

void sasCamera::getRight( smVec4& right ) const
{
  right.W = 0.f;
  smCross( _front, _up, right );
}

void sasCamera::getViewMatrix( smMat44& m ) const
{
  smVec4 right;
  getRight( right );
  smVec4 invFront;
  smInverse3( _front, invFront );
  invFront.W = 0.f;
  m.setColumn4<0>( right );
  m.setColumn4<1>( _up );
  m.setColumn4<2>( invFront );
  smInverse3( m, m );
  smVec4 invTrans;
  smInverse3( _position, invTrans );
  m.L0[3] = smDot3( m.L0, invTrans );
  m.L1[3] = smDot3( m.L1, invTrans );
  m.L2[3] = smDot3( m.L2, invTrans );
  m.L3[3] = 1.f;
}

void sasCamera::getProjMatrix( float width, float height, smMat44& m ) const
{   
  float xScale = smATan( _fovX / 2.f );
  float aspectRatio = width / height;
  float yScale = xScale * aspectRatio;  

  m.set(
    xScale, 0.f, 0.f, 0.f,
    0.f, yScale, 0.f, 0.f,
    0.f, 0.f, _zFar/(_zNear-_zFar), (_zNear*_zFar)/(_zNear-_zFar),
    0.f, 0.f, -1.f, 0.f );
}

void sasCamera::getProjMatrix_Ortho( float width, float height, smMat44& m ) const
{   
  m.set(
    2/width, 0.f, 0.f, 0.f,
    0.f, 2/height, 0.f, 0.f,
    0.f, 0.f, 1/(_zNear-_zFar), (_zNear)/(_zNear-_zFar),
    0.f, 0.f, 0.f, 1.f );
}
