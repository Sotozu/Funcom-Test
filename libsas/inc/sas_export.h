#pragma once

#define SAS_STATICLIB

#if defined( _WIN32 ) && !defined ( SAS_STATICLIB )
  #ifdef sasEXPORT
    #define sasAPI __declspec(dllexport) 
  #else
    #define sasAPI __declspec(dllimport)
  #endif
#else
  #define sasAPI
#endif
