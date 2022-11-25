#pragma once

#include "ss_export.h"
#include "sas_atomic.h"

class ssAPI sasSpinLock
{
  sasMEM_OP( sasSpinLock );
public:
  sasSpinLock();
  ~sasSpinLock();

  sasAtomic _lock;
};

ssAPI void sasLockSpinLock( sasSpinLock& lock, uint32_t spinsBeforeSleep = 50 );
ssAPI void sasUnlockSpinLock( sasSpinLock& lock );

class ssAPI sasScopedSpinLock
{
  sasMEM_OP( sasScopedSpinLock );
public:
  sasScopedSpinLock( sasSpinLock& lock ) { sasLockSpinLock( lock ); _lock = &lock; }
  ~sasScopedSpinLock()                   { sasUnlockSpinLock( *_lock ); }

private:
  sasSpinLock* _lock;
};

