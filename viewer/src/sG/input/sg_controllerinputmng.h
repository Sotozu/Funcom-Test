#pragma once

enum sgControllerButtons
{
  SG_CONTROLLER_BUTTON_A = 0,
  SG_CONTROLLER_BUTTON_B, 
  SG_CONTROLLER_BUTTON_X, 
  SG_CONTROLLER_BUTTON_Y, 
  SG_CONTROLLER_BUTTON_LS,
  SG_CONTROLLER_BUTTON_RS,
  SG_CONTROLLER_BUTTON_DUP,
  SG_CONTROLLER_BUTTON_DRIGHT,
  SG_CONTROLLER_BUTTON_DLEFT,
  SG_CONTROLLER_BUTTON_DDOWN,
  SG_CONTROLLER_BUTTON_LEFTTHUMB,
  SG_CONTROLLER_BUTTON_RIGHTTHUMB,
  SG_CONTROLLER_BUTTON_BACK,
  SG_CONTROLLER_BUTTON_START,

  SG_CONTROLLER_NUM_BUTTONS
};

struct sgControllerInput
{
  bool  _buttonStates[ SG_CONTROLLER_NUM_BUTTONS ];
  float _leftStickX;
  float _leftStickY;
  float _rightStickX;
  float _rightStickY;
  float _leftTrigger;
  float _rightTrigger;

  sgControllerInput() { clear(); }
  void clear();
};

class sgControllerInputMng
{
  sasNO_COPY( sgControllerInputMng );
  sasMEM_OP( sgControllerInputMng );

public:
  sgControllerInputMng();
  ~sgControllerInputMng();

  bool                      isControllerAvailable() const;
  const sgControllerInput&  controllerState() const;
  void                      vibrate( float leftIntensity, float rightIntensity );
  void                      setControllerInvertedY( bool state )  { _controllerInvertedY = state; }

  void step();

private:
  sgControllerInput _input;
  bool _controllerPresent;
  bool _controllerInvertedY;
};