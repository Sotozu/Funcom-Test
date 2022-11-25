#pragma once

#include "ss_export.h"
#include <windows.h>

class ssAPI sasSemaphore
{
  sasMEM_OP( sasSemaphore );
public:
  sasSemaphore();
  ~sasSemaphore();

  HANDLE _semaphore;
};

ssAPI void sasSignalSemaphore( sasSemaphore& semaphore );
ssAPI void sasWaitForSemaphore( sasSemaphore& mutex );


