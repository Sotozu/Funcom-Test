#include "sg_pch.h"

#include "input/sg_inputmng.h"
#include "utils/sas_stringid.h"
#include "utils/sas_path.h"
#include "sg_gameMng.h"
#include <time.h>

#if !FINAL

  //required for retrieving paths for dropped files for editor
  #include "Shellapi.h"
  #pragma comment( lib, "Shell32" )

#endif

// ============================================================================

LRESULT CALLBACK    wndProc( HWND h, UINT msg, WPARAM w, LPARAM l );
HWND                createWindow();
void                mainLoop();
void                update();

// ============================================================================

struct Global
{
  sgInputBlob _inputBlob;
  sgGameMng*  _gameMng;

  Global()
  {
    _gameMng = NULL;
  }
} G;


bool loadPathFromCfg( std::string& path )
{
  FILE* file;
  const char* mode = "r";

  file = fopen( "saspathconfig.cfg", mode);
  if( file == NULL )
  {
    path = "";
    return false;
  }

  fseek( file , 0 , SEEK_END);
  size_t size = ftell( file );
  rewind( file );

  path.resize( size );
  int s = fread( (uint8_t*)path.c_str(), 1, size, file ); 
  fclose( file );
  return s!= 0;
}

bool resolveRscPath( std::string& rscPath )
{
  std::string testPath = ".\\rsc\\";
  if( sasPathDoesFolderExist( testPath.c_str() ) )
  {
    rscPath = testPath;
    return true;
  }

  testPath = ".\\..\\rsc\\";
  if( sasPathDoesFolderExist( testPath.c_str() ) )
  {
    rscPath = testPath;
    return true;
  }
  
   testPath = ".\\..\\..\\rsc\\";
  if( sasPathDoesFolderExist( testPath.c_str() ) )
  {
    rscPath = testPath;
    return true;
  }

  if( loadPathFromCfg( testPath ) )
  {
    if( sasPathDoesFolderExist( testPath.c_str() ) )
    {
      rscPath = testPath;
      return true;
    }
  }

  OutputDebugString( "Can't find resources folder...\n" );
  return false;
}


// ============================================================================

int main( int argc, char** argv )
{
#ifdef PARANOID 
  _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF));
  // _CrtSetBreakAlloc( 2720 );
#endif

  HWND hwnd = createWindow();
  if( hwnd )
  {
    //init config
    sasConfig config;
    config.hwnd = hwnd;
    config.shaderFolder = sas_SHADERDIR;
    config.resourcePath = sas_RSCDIR;

    //look for path config file
    std::string pathCfg;
    std::string shaderPath;

    if( resolveRscPath( pathCfg ) )
    {
      shaderPath = pathCfg + "shaders/";
      config.shaderFolder = shaderPath.c_str();
      config.resourcePath = pathCfg.c_str();
    }

    srand( static_cast< unsigned int >( time( NULL ) ) );

    G._gameMng = new sgGameMng();
    G._gameMng->initialize( hwnd, config );

    G._gameMng->loadScene( sasStringId::invalid() );

    mainLoop();
        
    G._gameMng->destroy();
    delete G._gameMng;
  }  
  return 0;
}


LRESULT CALLBACK wndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
  sasInput input;

  input.hwnd = hwnd;
  input.msg = msg;
  input.lParam = lparam;
  input.wParam = wparam;

  G._inputBlob._ignoreInput |= sasApplyInput( input );

  switch( msg )
  {
  case WM_MOUSEMOVE: 
    {
      G._inputBlob._mouseX = LOWORD(lparam); 
      G._inputBlob._mouseY = HIWORD(lparam);
    }
    break;

  case WM_LBUTTONDOWN:
    {
      G._inputBlob._mouseLeftBtnDown = true;
    }
    break;

  case WM_LBUTTONUP:
    {
      G._inputBlob._mouseLeftBtnDown = false;
    }
    break;

  case WM_RBUTTONDOWN:
    {
      G._inputBlob._mouseRightBtnDown = true;
    }
    break;

  case WM_RBUTTONUP:
    {
      G._inputBlob._mouseRightBtnDown = false;
    }
    break;

  case WM_DESTROY: PostQuitMessage( 0 ); break;  
  default: return DefWindowProc( hwnd, msg, wparam, lparam );
  }
  return 0;
}


HWND createWindow()
{ 
  int desiredWidth = 1280;
  int desiredHeight = 720;

  HWND hwnd = 0;
  WNDCLASS wc = { 0 };

  wc.hInstance =  GetModuleHandle( 0 );
  wc.lpfnWndProc = wndProc;
  wc.lpszClassName = "DM";
  wc.style = CS_VREDRAW|CS_HREDRAW;

  if( RegisterClass( &wc ) )
  {
#if FINAL
    hwnd = CreateWindow( wc.lpszClassName, "sasViewer", WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, desiredWidth, desiredHeight, 0, 0, wc.hInstance, 0 );
#else
    hwnd = CreateWindowEx( WS_EX_ACCEPTFILES, wc.lpszClassName, "sasViewer", WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, desiredWidth, desiredHeight, 0, 0, wc.hInstance, 0 );
#endif
  }

  //adjust size to make room for potential borders
  RECT rcClient, rcWind;
  POINT ptDiff;
  GetClientRect( hwnd, &rcClient );
  GetWindowRect( hwnd, &rcWind );
  ptDiff.x = ( rcWind.right - rcWind.left ) - rcClient.right;
  ptDiff.y = ( rcWind.bottom - rcWind.top ) - rcClient.bottom;
  MoveWindow( hwnd, rcWind.left, rcWind.top, desiredWidth + ptDiff.x, desiredHeight + ptDiff.y, TRUE );

  GetClientRect( hwnd, &rcClient );
  GetWindowRect( hwnd, &rcWind );
  ptDiff.x = ( rcWind.right - rcWind.left ) - rcClient.right;
  ptDiff.y = ( rcWind.bottom - rcWind.top ) - rcClient.bottom;
  return hwnd;
}

void mainLoop()
{
  MSG msg;
  int shouldExit = 0;

  while( !shouldExit )
  {
    if( PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
    {      
      shouldExit = msg.message == WM_QUIT;
      if( !shouldExit )
      {        
        TranslateMessage(&msg);
        DispatchMessage(&msg);        
      }    
    }
    else 
    {
      update();
    }
  }
}

void update()
{
  G._gameMng->inputMng()->setInputBlob( G._inputBlob );
  G._inputBlob._ignoreInput = false;

  G._gameMng->step();
}

