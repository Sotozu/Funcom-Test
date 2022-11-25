// Need header guards rather than #pragma once here, to get precompiled headers working with make.
#ifndef SAS_PCH_H
#define SAS_PCH_H

#include "sas_main.h"
#include "su_main.h"

#ifdef PLATFORM_DX11
  #include <d3d11.h>
  #ifdef USE_D3DX
    #include <d3dx11.h>
  #else
    #include <D3DCompiler.h>
  #endif
#endif

#ifdef PLATFORM_OPENGL
  #include <cfloat>

  #include <GL/glew.h>
  #include <GL/glfw.h>
#endif

#define TW_NO_LIB_PRAGMA
#include <AntTweakBar.h>


#ifdef PLATFORM_OPENGL
  #ifndef _WIN32
    //extern const GLubyte * gluErrorString (GLenum error);
  #endif
  
  #ifndef NDEBUG
    #define printOpenGLError() printOglError(__FILE__, __LINE__)
    int printOglError(const char *file, int line);
  #else
    #define printOpenGLError() do {} while(0);
  #endif
#endif

#include "sys/sas_mem.h"
#include "sas_cmp.h"
#include "sys/sas_stackalloc.h"
#include "utils/sas_pix.h"
#include "render/sas_rendertypes.h"

#endif // SAS_PCH_H