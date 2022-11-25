#include "sg_pch.h"
#include "sg_cameraMng.h"
#include "input/sg_inputmng.h"

sgCameraMng::sgCameraMng()
  : _current( -1 )
  , _runningCameraPath( NULL )
{
}

sgCameraMng::~sgCameraMng()
{
  sasASSERT( _cameras.empty() );
}

void sgCameraMng::addCamera( sasCamera* camera )
{
  sasASSERT( getCameraIndex( camera ) == -1 );
  _cameras.push_back( camera );
  if( _cameras.size() == 1 )
    _current = 0;
}

void sgCameraMng::removeCamera( sasCamera* camera )
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

sasCamera* sgCameraMng::getCurrentCamera()
{
  if( _current == -1 )
    return 0;  
  return _cameras[ _current ];
}

int sgCameraMng::getCameraIndex( sasCamera* camera )
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

void sgCameraMng::step( sgInputMng* inputMng, float deltaTime )
{
  if( _runningCameraPath != nullptr )
  {
  }
  else
  {

    sasCamera* currentCam = _cameras[ _current ];
    {
      //free cam update based on inputs
      if( currentCam )
      {
        static smVec4 pos = sasCameraPosition( currentCam );
        smVec4 front      = sasCameraFront( currentCam );
        smVec4 right      = sasCameraRight( currentCam );
        smVec4 up         = sasCameraUp( currentCam );
        smVec4 dpos( 0.f );
        static float rotUp = 45.f;
        static float rotRight = 0.f;
        static float headRotUp = 0.f;
        static float headRotRight = 0.f;
        static float headRotSideTilt = 0.f;

        {
          if( inputMng->isKeyPressed( 'W' ) )   
            dpos += front * 8.f;
          if( inputMng->isKeyPressed( 'S' ) )   
            dpos -= front * 8.f;
          if( inputMng->isKeyPressed( 'A' ) )   
            dpos -= right * 8.f;
          if( inputMng->isKeyPressed( 'D' ) )  
            dpos += right * 8.f;
          if( inputMng->isKeyPressed( sgSpecialKeys::Left ) )   
            rotUp -= 100.f * deltaTime;
          if( inputMng->isKeyPressed( sgSpecialKeys::Right ) )  
            rotUp += 100.f * deltaTime;
          if( inputMng->isKeyPressed( sgSpecialKeys::Up ) )   
            rotRight += 100.f * deltaTime;
          if( inputMng->isKeyPressed( sgSpecialKeys::Down ) )  
            rotRight -= 100.f * deltaTime;


          if( inputMng->isKeyPressed( 'G' ) )   
            headRotRight -= 100.f * deltaTime;
          if( inputMng->isKeyPressed( 'J' ) ) 
            headRotRight += 100.f * deltaTime;
          if( inputMng->isKeyPressed( 'T' ) )
            headRotSideTilt -= 100.f * deltaTime;
          if( inputMng->isKeyPressed( 'U' ) ) 
            headRotSideTilt += 100.f * deltaTime;
          if( inputMng->isKeyPressed( 'H' ) )  
            headRotUp += 100.f * deltaTime;
          if( inputMng->isKeyPressed( 'Y' ) )  
            headRotUp -= 100.f * deltaTime;


          if( inputMng->mouseState( sgMouse::RightBtn ) )
          {
            float invertedCoeff = sasGetMouseInverted() ? -1.f : 1.f;
            rotUp += 80.f * deltaTime * inputMng->mouseDX();
            rotRight += 80.f * deltaTime * inputMng->mouseDY() * invertedCoeff;
          }

          
        }

        // controller input
        if( inputMng->controllerMng()->isControllerAvailable() )
        {
          const sgControllerInput controllerState = inputMng->controllerMng()->controllerState();

          if( controllerState._leftStickY )
            dpos += front * 10.f * controllerState._leftStickY;

          if( controllerState._leftStickX )
            dpos += right * 10.f * controllerState._leftStickX;

          if( controllerState._rightStickX )
            rotUp += 100.f * deltaTime * controllerState._rightStickX;

          if( controllerState._rightStickY )
            rotRight += 100.f * deltaTime * controllerState._rightStickY;
        }

        pos += deltaTime * dpos;
        rotRight = smClamp( rotRight, -89, 89 );   
        rotUp = smNormalizeAngle( rotUp );

        headRotUp = smClamp( headRotUp, -45.f, 45.f ); 
        headRotRight = smClamp( headRotRight, -45.f, 45.f ); 
        headRotSideTilt = smClamp( headRotSideTilt, -179.f, 179.f ); 

        float ydt = 0.f;
        smVec3 posOffset( 0.f );
        inputMng->ovrMng()->getOVRInput( deltaTime, ydt, headRotRight, headRotUp, headRotSideTilt, posOffset );

        smMat44 mHeadRotFinal;
        smEulerAngles headAngles;
        headAngles.Pitch = headRotUp;
        headAngles.Yaw = headRotRight;
        headAngles.Roll = headRotSideTilt;
        smGetXFormTRSMatrix( 0.f, 0.f, 0.f, headAngles, 1.f, 1.f, 1.f, mHeadRotFinal );


        smVec4 lookAt( 0,-1,0,0 );
        smVec4 upVec( 0.f, 0.f, -1.f, 0.f );
        smMat44 mRotRight;
        smMat44 mRotUp;
        smMat44 mRotFinal;
        smGetRotIMatrix( rotRight, mRotRight );
        smGetRotJMatrix( rotUp, mRotUp );
        smMul3( mRotUp, mRotRight, mRotFinal );
        //head tracking
        smMul3( mHeadRotFinal, lookAt, lookAt );
        smMul3( mHeadRotFinal, upVec, upVec );
        //body tracking
        smMul3( mRotFinal, lookAt, lookAt );  
        smMul3( mRotFinal, upVec, upVec );

        smVec4 posOffset4 = posOffset;
        posOffset4.W = 0.f;
        smMul3( mRotFinal, posOffset4, posOffset );

        //added neck joint
        const float neckToEyesLength = 0.165f;
        smVec3 finalPos = pos + posOffset + upVec * neckToEyesLength;
        smAdd3( finalPos, lookAt, lookAt );  

        sasCameraSetPosition( finalPos, currentCam );
        sasCameraLookAt( lookAt, upVec, currentCam );
      }

    }

  }
}

void sgCameraMng::runCameraPath( sgCameraPath* cameraPath, bool looping, float speed )
{
}

void sgCameraMng::stopCameraPath()
{
}

void sgCameraMng::attachCamera( sasCamera* camera, sg::gamecode::Player* target )
{
}

bool sgCameraMng::isCameraBound( sasCamera* camera ) const
{
    return false;
}
