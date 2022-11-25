#include "sas_pch.h"
#include "sas_pix.h"

#ifdef PLATFORM_DX11

#include <d3d9.h>
#pragma comment( lib, "d3d9" )

sasPixEvent::sasPixEvent( const char* name, DWORD colour )
{
  std::string str(name); 
  std::wstring str2(str.length(), L' ');
  std::copy(str.begin(), str.end(), str2.begin());

  D3DPERF_BeginEvent( colour, str2.c_str());
  D3DPERF_SetMarker( colour, str2.c_str());
}

sasPixEvent::~sasPixEvent()
{
  D3DPERF_EndEvent();
}


#endif