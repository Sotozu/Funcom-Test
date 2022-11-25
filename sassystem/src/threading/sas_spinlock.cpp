#include "ss_pch.h"
#include "threading/sas_spinlock.h"


sasSpinLock::sasSpinLock()
  : _lock( 0 )
{
}

sasSpinLock::~sasSpinLock()
{
  sasASSERT( _lock == 0 );
}


void sasLockSpinLock( sasSpinLock& lock, uint32_t spinsBeforeSleep )
{
  uint32_t spins = 0;
  while( sasAtomicCmpAndExchange( &( lock._lock ), 1, 0 ) != 0 )
  {
    ++spins;
    if( spins > spinsBeforeSleep )
      Sleep( 0 );
  }
}

void sasUnlockSpinLock( sasSpinLock& lock )
{
  sasAtomicExchange( &( lock._lock ), 0 );
}

