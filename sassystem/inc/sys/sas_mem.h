#pragma once 

#include <map>

// ----------------------------------------------------------------------------
//  malloc/free
// ----------------------------------------------------------------------------

#define sasMalloc(S)        gMemMng->malloc( __FILE__, __LINE__, 0, (S) )
#define sasFree(P)          gMemMng->free( P )

// ----------------------------------------------------------------------------
//  new/delete
// ----------------------------------------------------------------------------

#define sasMEM_OP(C)         \
  public: void* operator new( size_t s, const char* f, int l )    { return gMemMng->malloc( f, l, #C, s );  } \
  public: void  operator delete(void* p, const char* f, int l )   { gMemMng->free(p); } \
  public: void  operator delete(void* p)                          { gMemMng->free(p); } \
  public: void* operator new[]( size_t s, const char* f, int l )  { return gMemMng->malloc( f, l, #C, s );  } \
  public: void  operator delete[]( void* p, const char* f, int l ){ gMemMng->free(p); } \
  public: void  operator delete[]( void* p)                       { gMemMng->free(p); } \

#define sasNew              new ( __FILE__, __LINE__ )
#define sasDelete           delete

// ----------------------------------------------------------------------------
//  sasMemoryMng
// ----------------------------------------------------------------------------

class sasScratchMemoryMng;

#include "ss_export.h"

class sasMemoryMng
{
public:
  sasMemoryMng();
  ~sasMemoryMng();

public:
  ssAPI void* malloc( const char* file, int line, const char* type, size_t size );
  ssAPI void  free( void* ptr );
  ssAPI void  dumpAllocs();

private:
  struct Entry 
  {
    size_t      size;
    const char* file;
    int         line;
    const char* type;    
  };
  typedef std::map< void*, Entry > Allocations;

  Allocations _allocations;
};

ssAPI extern sasMemoryMng* gMemMng;
