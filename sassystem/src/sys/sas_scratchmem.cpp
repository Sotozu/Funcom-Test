#include "ss_pch.h"
#include "sys/sas_scratchmem.h"

#define MIN_BLOCK_SIZE 128

sasScratchMemoryMng::sasScratchMemoryMng()
{
  //allocate scratch memory blocks

  size_t allocationSize = MIN_BLOCK_SIZE; 
  size_t totalAllocatedMemory = 0;
  for( unsigned int i = 0; i < 17; i++ )
  {
    Entry block;
    block._size = allocationSize;
    block._state = FALSE;
    block._memBlock = gMemMng->malloc( "scratchMem", 0, "void", allocationSize );
    _preallocatedBlocks[ block._memBlock ] = block;

    totalAllocatedMemory += allocationSize;
    allocationSize *= 2;
  }
  sasDebugOutput( "sasScratchMemoryMng: allocated %d, max block size: %d\n", totalAllocatedMemory, allocationSize / 2 );
}

sasScratchMemoryMng::~sasScratchMemoryMng()
{
  sasScopedSpinLock lock( _lock );
  Allocations::iterator it = _preallocatedBlocks.begin();
  while( it != _preallocatedBlocks.end() )
  {
    gMemMng->free( it->second._memBlock );
    it++;
  }
  _preallocatedBlocks.clear();
}

unsigned int getNextPowerOfTwo( unsigned int n )
{
  n--;
  n |= n >> 1;   
  n |= n >> 2;   
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  return n++;  
}

void* sasScratchMemoryMng::allocateScratchMemory( int size )
{
  sasScopedSpinLock lock( _lock );
  Allocations::iterator it = _preallocatedBlocks.begin();
  while( it != _preallocatedBlocks.end() )
  {
    if( ( it->second._size >= size ) && ( it->second._state == FALSE ) )
    {
      it->second._state = TRUE;
      return it->second._memBlock;
    }
    it++;
  }
  sasDebugOutput( "sasScratchMemoryMng::allocateScratchMemory: failed to find free block large enough for requested %d bytes...", size );
  sasASSERT( false );
  return NULL;
}

void sasScratchMemoryMng::freeScratchMemory( void* ptr )
{
  sasScopedSpinLock lock( _lock );
  sasASSERT( _preallocatedBlocks[ ptr ]._state == TRUE );
  _preallocatedBlocks[ ptr ]._state = FALSE;
}
