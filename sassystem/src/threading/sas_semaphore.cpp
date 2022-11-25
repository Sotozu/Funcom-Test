#include "ss_pch.h"
#include "threading/sas_semaphore.h"

sasSemaphore::sasSemaphore()
{
  _semaphore = CreateSemaphore( NULL, 0, 1, NULL );
  sasASSERT( _semaphore != NULL );
}

sasSemaphore::~sasSemaphore()
{
  CloseHandle( _semaphore );
}

void sasSignalSemaphore( sasSemaphore& semaphore )
{
  ReleaseSemaphore( semaphore._semaphore, 1, NULL );
}

void sasWaitForSemaphore( sasSemaphore& semaphore )
{
  WaitForSingleObject( semaphore._semaphore, INFINITE );
}


