#include "sg_pch.h"
#include "sg_inputmng.h"

sgInputMng::sgInputMng()
  : _controllerMng( NULL )
  , _mouseDX( 0 )
  , _mouseDY( 0 )
  , _mouseX( 0 )
  , _mouseY( 0 )
  , _leftMouseDown( false )
  , _rightMouseDown( false )
{
  _controllerMng = sasNew sgControllerInputMng();
  _ovrMng = sasNew sgOvrInputMng();

  _inputBlob._mouseX = _inputBlob._mouseY = -1;
  _inputBlob._mouseLeftBtnDown = _inputBlob._mouseRightBtnDown = false;

}

sgInputMng::~sgInputMng()
{
  sasDelete _ovrMng;
  sasDelete _controllerMng;
}

void sgInputMng::step()
{
  _ovrMng->step();

  // update pad state
  _controllerMng->step();

  // update mouse state
  _mouseDX = _inputBlob._mouseX - _mouseX;
  _mouseDY = _inputBlob._mouseY - _mouseY;
  _mouseX = _inputBlob._mouseX;
  _mouseY = _inputBlob._mouseY;
  _leftMouseDown = _inputBlob._mouseLeftBtnDown;
  _rightMouseDown = _inputBlob._mouseRightBtnDown;

  // update keyboard state
  GetKeyboardState( _keyboardState );
}

bool sgInputMng::isKeyPressed( BYTE k ) const
{
  if( _inputBlob._ignoreInput )
    return false;

  if( _keyboardState[ k ] & 0x80 )
    return true;
  else
    return false;
}

BYTE convertToWindowsKeyCode( sgSpecialKeys::Enum k )
{
  switch( k )
  {
  case sgSpecialKeys::Ctrl:
    return VK_CONTROL;

  case sgSpecialKeys::Shift:
    return VK_SHIFT;

  case sgSpecialKeys::Esc:
    return VK_ESCAPE;

  case sgSpecialKeys::Down:
    return VK_DOWN;

  case sgSpecialKeys::Up:
    return VK_UP;

  case sgSpecialKeys::Left:
    return VK_LEFT;

  case sgSpecialKeys::Right:
    return VK_RIGHT;

  default:
    sasASSERT( false );
    return 0;
  }
}

bool sgInputMng::isKeyPressed( sgSpecialKeys::Enum k ) const
{
  if( _inputBlob._ignoreInput )
    return false;

  if( _keyboardState[ convertToWindowsKeyCode( k ) ] & 0x80 )
    return true;
  else
    return false;
}

bool sgInputMng::mouseState( sgMouse::Enum m ) const
{
  if( _inputBlob._ignoreInput )
    return false;

  if( m == sgMouse::LeftBtn )
    return _leftMouseDown;
  if( m == sgMouse::RightBtn )
    return _rightMouseDown;

  return false;
}