#include "sas_pch.h"
#include "sas_gui.h"
#include "sas_device.h"
#include "resource/sas_splashscreen.h"
#include "render/sas_rendermng.h"

void TW_CALL CopyStdStringToClient( std::string& dst, const std::string& src )
{
  dst = src;

}
sasGui::sasGui()
{
#ifdef PLATFORM_DX11
  TwInit(TW_DIRECT3D11, sasDevice::singleton().d3dDevice() );
#else
  TwInit(TW_OPENGL_CORE, NULL);
  while( glGetError() != GL_NO_ERROR)
  {
    printf("Anttweakbar: GL error on init!\n");
  }

#endif

  //needed for atb to handle std strings
  TwCopyStdStringToClientFunc( CopyStdStringToClient );

  resizeWindow();
}

sasGui::~sasGui()
{
  TwTerminate();
}

void sasGui::render()
{	
#ifndef PLATFORM_DX11
  printOpenGLError();
#endif

  if( !sasRenderMng::singletonPtr()->uiDisabled() && sasSplashScreen::getActiveSplashScreen() == NULL )
    TwDraw();

#ifndef PLATFORM_DX11
  while( glGetError() != GL_NO_ERROR)
  {
    printf("Anttweakbar: GL error on render!\n");
  }
#endif
}

bool sasGui::applyInput( const sasInput& input )
{
#ifdef _WIN32
  int res = TwEventWin( input.hwnd, input.msg, input.wParam, input.lParam );
  return ( res != 0 ); 
#endif
}

void sasGui::resizeWindow()
{	
  size_t w = sasDevice::singleton().backBufferWidth();
  size_t h = sasDevice::singleton().backBufferHeight();
  TwWindowSize( w, h );  
}

