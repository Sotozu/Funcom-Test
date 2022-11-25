#pragma once

#ifdef _WIN32
  #ifdef sasUtilsEXPORT
    #define suAPI __declspec(dllexport) 
  #else
    #define suAPI __declspec(dllimport)
  #endif
#else
  #define suAPI
#endif
