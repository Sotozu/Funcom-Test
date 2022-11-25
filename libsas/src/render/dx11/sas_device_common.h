#pragma once 

#define DX(X)           do{ hr = (X); if FAILED(hr) { __asm int 3 } } while(0)
#define DX_RELEASE(X)   do{ if(X) { (X)->Release(); (X)=0; } } while(0)
