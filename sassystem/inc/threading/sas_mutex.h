#pragma once

#include "ss_export.h"
#include <windows.h>

class ssAPI sasMutex
{
  sasMEM_OP( sasMutex );
public:
  sasMutex();
  ~sasMutex();

  HANDLE _mutex;
};

ssAPI void sasLockMutex( sasMutex& mutex );
ssAPI void sasUnlockMutex( sasMutex& mutex );

class ssAPI sasScopedMutexLock
{
  sasMEM_OP( sasScopedMutexLock );
public:
  sasScopedMutexLock( sasMutex& mutex ) { sasLockMutex( mutex ); _mutex = &mutex; }
  ~sasScopedMutexLock()                 { sasUnlockMutex( *_mutex ); }

private:
  sasMutex* _mutex;
};

