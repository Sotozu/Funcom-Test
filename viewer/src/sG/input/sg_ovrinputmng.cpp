#include "sg_pch.h"
#include "sg_ovrinputmng.h"

sgOvrInputMng::sgOvrInputMng()
  : _ovrHMD( nullptr )
  , _ovrDisplayId( 0xffffffff )
{
  initializeOVR();  
}

sgOvrInputMng::~sgOvrInputMng()
{
  shutdownOVR();
}

void sgOvrInputMng::step()
{
  if( _ovrHMD )
  {
    ovrHmd_BeginFrame( _ovrHMD, 0 );
    ovrHmd_DismissHSWDisplay( _ovrHMD );
  }
}

bool sgOvrInputMng::initializeOVR()
{
  ovr_Initialize();
  //return false;
  _ovrHMD = ovrHmd_Create( 0 );
  if( !_ovrHMD )
  {
    OutputDebugString( "Oculus Rift not detected.\n" );
    return false;
  }
  if( _ovrHMD->ProductName[0] == '\0' ) 
  {
    OutputDebugString( "Oculus Rift detected, display not enabled.\n" );
    ovrHmd_Destroy( _ovrHMD );
    _ovrHMD = nullptr;
    return false;
  }

  ovrHmd_SetEnabledCaps( _ovrHMD, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction );
  ovrHmd_ConfigureTracking( _ovrHMD, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0 );

  _ovrDisplayId = _ovrHMD->DisplayId;
  _ovrDisplayName = _ovrHMD->DisplayDeviceName;
  return true;
}

void sgOvrInputMng::shutdownOVR()
{
  if( _ovrHMD )
  {
    ovrHmd_Destroy( _ovrHMD );
    _ovrHMD = nullptr;
  }

  ovr_Shutdown(); 
}

bool sgOvrInputMng::getOVRInput( float dt, float& yawDelta, float& yaw, float& pitch, float& roll, smVec3& posOffset )
{
  //approximate mono position and orientation for camera for non rendering reasons
  if( _ovrHMD )
  { 
    ovrPosef leftPose = ovrHmd_GetHmdPosePerEye( _ovrHMD, ovrEye_Left );
    ovrPosef rightPose = ovrHmd_GetHmdPosePerEye( _ovrHMD, ovrEye_Right );

    //position
    smVec3 leftPosition = smVec3( leftPose.Position.x, leftPose.Position.y, leftPose.Position.z );
    smVec3 rightPosition = smVec3( rightPose.Position.x, rightPose.Position.y, rightPose.Position.z );
    posOffset = ( leftPosition + rightPosition ) * 0.5f;
    //sasDebugOutput( "headOffset %f %f %f \n", posOffset.X, posOffset.Y, posOffset.Z );

    //orientation
    static float lastSensorYaw = 0.f;
    OVR::Quatf qOrientation = leftPose.Orientation;
    qOrientation.GetEulerAngles< OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z >( &yaw, &pitch, &roll );
    yaw = -1.f * smRad2Deg( yaw );
    pitch = -1.f * smRad2Deg( pitch );
    roll = -1.f * smRad2Deg( roll );
    yawDelta = ( yaw - lastSensorYaw );
    lastSensorYaw = yaw;

    return true;
  } 

  return false;
}
