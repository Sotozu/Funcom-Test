#pragma once

#ifdef _WIN32
  #ifdef sasSystemsEXPORT
    #define ssAPI __declspec(dllexport) 
  #else
    #define ssAPI __declspec(dllimport)
  #endif
#else
  #define ssAPI
#endif
