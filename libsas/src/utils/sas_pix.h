#pragma once

#ifdef PLATFORM_DX11

class sasPixEvent
{
public:
  sasPixEvent( const char* name, DWORD colour );
  ~sasPixEvent();

};

#define INSERT_PIX_EVENT( name, colour ) sasPixEvent pixEvent( name, colour );

#else

#define INSERT_PIX_EVENT

#endif



