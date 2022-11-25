#pragma once

#include "sg_controllerinputmng.h"
#include "sg_ovrinputmng.h"
#include "utils/sas_singleton.h"

namespace sgSpecialKeys
{
  enum Enum
  {
    Ctrl = 0,
    Shift,
    Esc,
    Left,
    Right,
    Up,
    Down,
  };
}

namespace sgMouse
{
  enum Enum
  {
    LeftBtn = 0,
    RightBtn,
  };
}

struct sgInputBlob
{
  int   _mouseX;
  int   _mouseY;
  bool  _mouseLeftBtnDown;
  bool  _mouseRightBtnDown;
  bool  _ignoreInput;
};

class sgInputMng : public sasSingleton< sgInputMng >
{
  sasNO_COPY( sgInputMng );
  sasMEM_OP( sgInputMng );

public:
  sgInputMng();
  ~sgInputMng();

  void step();

  sgControllerInputMng* controllerMng() const { return _controllerMng; }
  sgOvrInputMng*        ovrMng()              { return _ovrMng; }

  bool                  isKeyPressed( BYTE k ) const;
  bool                  isKeyPressed( sgSpecialKeys::Enum k ) const;

  int                   mouseX() const  { return _mouseX; }
  int                   mouseY() const  { return _mouseY; }
  int                   mouseDX() const { return _mouseDX; }
  int                   mouseDY() const { return _mouseDY; }
  bool                  mouseState( sgMouse::Enum m ) const;

  void setInputBlob( const sgInputBlob& inputBlob ) { _inputBlob = inputBlob; }

private:
  //pad input
  sgControllerInputMng* _controllerMng;

  //ovr input
  sgOvrInputMng*        _ovrMng;

  //keyboard input
  BYTE                  _keyboardState[ 256 ];

  //mouse input
  int                   _mouseX;
  int                   _mouseY;
  int                   _mouseDX;
  int                   _mouseDY;
  bool                  _leftMouseDown;
  bool                  _rightMouseDown;

  //internal
  sgInputBlob           _inputBlob;

};