#include "sg_pch.h"
#include "sg_inputmng.h"
#include "xinput.h"

void sgControllerInput::clear()
{

  for( unsigned int i = 0; i < SG_CONTROLLER_NUM_BUTTONS; i++ )
  {
    _buttonStates[ i ] = false;
  }

  _leftStickX = 0.f;
  _leftStickY = 0.f;
  _rightStickX = 0.f;
  _rightStickY = 0.f;
  _leftTrigger = 0.f;
  _rightTrigger = 0.f;
}

sgControllerInputMng::sgControllerInputMng()
{
  _input.clear();
  _controllerPresent = false;
  _controllerInvertedY = true;
}

sgControllerInputMng::~sgControllerInputMng()
{}

bool sgControllerInputMng::isControllerAvailable() const
{
  return _controllerPresent;
}
const sgControllerInput& sgControllerInputMng::controllerState() const
{
  return _input;
}

void sgControllerInputMng::vibrate( float leftIntensity, float rightIntensity )
{
  XINPUT_VIBRATION vibration;
  memset( &vibration, 0, sizeof( XINPUT_VIBRATION ) );

  vibration.wLeftMotorSpeed = static_cast< int >( leftIntensity * 65535.f );
  vibration.wRightMotorSpeed = static_cast< int >( rightIntensity * 65535.f );

  XInputSetState( 0, &vibration );
}

void sgControllerInputMng::step()
{
  static XINPUT_STATE xi_controller_state;

  memset( &xi_controller_state, 0, sizeof(XINPUT_STATE) );

  DWORD r = XInputGetState( 0, &xi_controller_state );
  _controllerPresent = ( r == ERROR_SUCCESS );


  _input.clear();

  if( _controllerPresent )
  {
    //triggers
    if( xi_controller_state.Gamepad.bLeftTrigger && xi_controller_state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD )
    {
      _input._leftTrigger = static_cast< float >( xi_controller_state.Gamepad.bLeftTrigger ) / 255.f;
    }

    if( xi_controller_state.Gamepad.bRightTrigger && xi_controller_state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD )
    {
      _input._rightTrigger = static_cast< float >( xi_controller_state.Gamepad.bRightTrigger ) / 255.f;
    }

    //buttons
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_A ) _input._buttonStates[ SG_CONTROLLER_BUTTON_A ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_B ) _input._buttonStates[ SG_CONTROLLER_BUTTON_B ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_Y ) _input._buttonStates[ SG_CONTROLLER_BUTTON_Y ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_X ) _input._buttonStates[ SG_CONTROLLER_BUTTON_X ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_START ) _input._buttonStates[ SG_CONTROLLER_BUTTON_START ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK ) _input._buttonStates[ SG_CONTROLLER_BUTTON_BACK ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN ) _input._buttonStates[ SG_CONTROLLER_BUTTON_DDOWN ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT ) _input._buttonStates[ SG_CONTROLLER_BUTTON_DLEFT ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT ) _input._buttonStates[ SG_CONTROLLER_BUTTON_DRIGHT ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP ) _input._buttonStates[ SG_CONTROLLER_BUTTON_DUP ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER ) _input._buttonStates[ SG_CONTROLLER_BUTTON_LS ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER ) _input._buttonStates[ SG_CONTROLLER_BUTTON_RS ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB ) _input._buttonStates[ SG_CONTROLLER_BUTTON_LEFTTHUMB ] = true;
    if( xi_controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB ) _input._buttonStates[ SG_CONTROLLER_BUTTON_RIGHTTHUMB ] = true;

    //sticks
    if( ( xi_controller_state.Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
          xi_controller_state.Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
        ( xi_controller_state.Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
          xi_controller_state.Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) )
    {    
      xi_controller_state.Gamepad.sThumbLX = 0;
      xi_controller_state.Gamepad.sThumbLY = 0;
    }

    _input._leftStickX = xi_controller_state.Gamepad.sThumbLX / 32767.f;
    _input._leftStickY = xi_controller_state.Gamepad.sThumbLY / 32767.f;
   
    if( ( xi_controller_state.Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
      xi_controller_state.Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) &&
      ( xi_controller_state.Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
      xi_controller_state.Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) )
    {    
      xi_controller_state.Gamepad.sThumbRX = 0;
      xi_controller_state.Gamepad.sThumbRY = 0;
    }

    _input._rightStickX = xi_controller_state.Gamepad.sThumbRX / 32767.f;
    _input._rightStickY = xi_controller_state.Gamepad.sThumbRY / 32767.f;

    if( !_controllerInvertedY )
    {
      _input._rightStickY *= -1.f;
    }
  }

}

