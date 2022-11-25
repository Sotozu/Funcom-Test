#pragma once 

#include "ss_export.h"
#include "threading/sas_spinlock.h"

class sasScratchMemoryMng
{
  sasMEM_OP( sasScratchMemoryMng );
public:
  sasScratchMemoryMng();
  ~sasScratchMemoryMng();

  ssAPI void* allocateScratchMemory( int size );
  ssAPI void  freeScratchMemory( void* ptr );

private:
  struct Entry 
  {
    void* _memBlock;
    int   _size;
    int   _state;
  };
  typedef std::map< void*, Entry > Allocations;
  Allocations _preallocatedBlocks;
  sasSpinLock _lock;
};

ssAPI extern sasScratchMemoryMng*  gScratchMemMng;
