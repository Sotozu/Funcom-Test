#include "ss_pch.h"
#include "sys/sas_mem.h"
#include "threading/sas_spinlock.h"

static sasSpinLock memMngLock;

sasMemoryMng::sasMemoryMng()
{
}

sasMemoryMng::~sasMemoryMng()
{
  dumpAllocs();  
}

void* sasMemoryMng::malloc( const char* file, int line, const char* type, size_t size )
{
#ifdef _MSC_VER
  void* ptr = _aligned_malloc( size, 16 );
#else
  void* ptr = NULL;
  posix_memalign( &ptr, 16, size );
#endif
  sasASSERT( ptr );

  sasScopedSpinLock lock( memMngLock );
  
  Entry entry;
  entry.size = size;
  entry.file = file;
  entry.line = line;
  entry.type = type; 
  _allocations.insert( std::make_pair( ptr, entry ) );
  return ptr;
}

void  sasMemoryMng::free( void* ptr )
{
  if( ptr != NULL )
  {
    {
      sasScopedSpinLock lock( memMngLock );
      Allocations::iterator i = _allocations.find( ptr );
      sasASSERT( i != _allocations.end() );
      _allocations.erase( i );
    }
    
#ifdef _MSC_VER
    _aligned_free( ptr );
#else
    ::free( ptr );
#endif
    
  }
}

void  sasMemoryMng::dumpAllocs()
{
  sasScopedSpinLock lock( memMngLock );
  Allocations::const_iterator i = _allocations.begin();
  Allocations::const_iterator last = _allocations.end();
  char dumpBuffer[ 1024 ];
  while( i!=last )
  {
    const Entry& e = (*i).second;
    sprintf( dumpBuffer, "%s(%d):memleak:%s\n", e.file, e.line, e.type );
    sasDebugOutput( dumpBuffer );
    ++i;
  }
}
