#include "ss_pch.h"
#include "ss_core.h"
#include "sys/sas_scratchmem.h"
#include "utils/sas_timer.h"
#include "threading/sas_jobmng.h"

ssAPI sasMemoryMng*           gMemMng;
ssAPI sasScratchMemoryMng*    gScratchMemMng;
ssAPI sasTimer*               gTimer;
ssAPI sasJobMng*              gJobMng;

void* jsonMalloc(size_t s )
{
  return sasMalloc( s );
}

void jsonFree(void* ptr )
{
  sasFree( ptr );
}

sasSystemCore::sasSystemCore()
  : _memoryMng(0)
  , _scratchMemMng(0)
  , _timer(0)
{  
  sasSTATIC_ASSERT( BOOTSTRAP_BUFFER_SIZE >= sizeof( sasMemoryMng ) );
  _memoryMng    = new( _boostrapBuffer )( sasMemoryMng )();
  gMemMng = _memoryMng;

  _scratchMemMng = sasNew sasScratchMemoryMng();
  gScratchMemMng = _scratchMemMng;

  json_set_alloc_funcs( jsonMalloc, jsonFree );

  _timer  = sasNew sasTimer();
  gTimer  = _timer;

  gJobMng = sasNew sasJobMng();
}

sasSystemCore::~sasSystemCore()
{
  // reverse order
  sasDelete gJobMng;
  gJobMng = NULL;

  sasDelete _timer;
  gTimer = NULL;

  sasDelete _scratchMemMng;
  gScratchMemMng = NULL;

  _memoryMng->~sasMemoryMng();
  gMemMng = NULL;
}

void sasSystemCore::update()
{
  _timer->update();
}