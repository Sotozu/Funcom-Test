#include "ss_pch.h"
#include "threading/sas_mutex.h"

sasMutex::sasMutex()
{
  _mutex = CreateMutex( NULL, FALSE, NULL );
  sasASSERT( _mutex != NULL );
}

sasMutex::~sasMutex()
{
  CloseHandle( _mutex );
}

void sasLockMutex( sasMutex& mutex )
{
  WaitForSingleObject( mutex._mutex, INFINITE );
}

void sasUnlockMutex( sasMutex& mutex )
{
  ReleaseMutex( mutex._mutex );
}

