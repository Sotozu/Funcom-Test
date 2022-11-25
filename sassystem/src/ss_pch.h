// Need header guards rather than #pragma once here, to get precompiled headers working with make.
#ifndef SASSYSTEM_PCH_H
#define SASSYSTEM_PCH_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <new>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <stdint.h>

#include <jansson.h>

#define sasNO_COPY(C)                  \
  private: C( const C& );             \
  private: C& operator=( const C& );  \

#define sasCONCAT_TOKEN(X,Y) X##Y
#define sasCONCAT(X,Y)      sasCONCAT_TOKEN(X,Y)
#define sasBREAK()          do { __asm int 3 } while(0)

#ifdef sasDEBUG
#define sasASSERTS_ENABLED
#endif

#ifdef sasASSERTS_ENABLED
#ifdef _MSC_VER
#define sasASSERT(C)        do { if(!(C)) { sasDebugOutput("ASSERT: %s (%s:%u)\n", #C, __FILE__, __LINE__); sasBREAK();  } } while(0)
#else
#define sasASSERT(C)        do { if(!(C)) { printf("ASSERT: %s (%s:%u)\n", #C, __FILE__, __LINE__); sasBREAK(); exit(0); } } while(0)
#endif
#else
#define sasASSERT(C)
#endif

#define sasSTATIC_ASSERT(C)  struct sasCONCAT(_STATIC_ASSERT_,__COUNTER__) { char buffer[ (C)?1:-1 ];  };

#ifdef _MSC_VER
#define sasALIGN16_PRE   __declspec( align(16) )
#define sasALIGN16_POST
#else
#define sasALIGN16_PRE
#define sasALIGN16_POST  __attribute__ ((aligned (16)))
#endif

#define sasALIGNED16(ADDR)  ((0xf & size_t(ADDR))==0)
#define sasALIGNON16(ADDR)  ((size_t(ADDR)+0xf)&(~0xf))

void sasDebugOutput(const char* szFormat, ...);

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#define alloca _alloca
#else
#include <stdint.h>
#endif


#include "ss_main.h"
#include "sys/sas_mem.h"
#include "sys/sas_stackalloc.h"
#include "sys/sas_scratchmem.h"

#endif // SASSYSTEM_PCH_H