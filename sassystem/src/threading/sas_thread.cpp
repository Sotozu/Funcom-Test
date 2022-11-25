#include "ss_pch.h"
#include "threading/sas_thread.h"
#include <process.h>

sasThread::sasThread()
  : _active( false )
  , _id( 0xffffffff )
  , _threadHandle( NULL )
{
}

sasThread::~sasThread()
{
  kill();
}

void sasThread::start( sasThreadFn fn, void* args )
{
  _threadHandle = (HANDLE)_beginthreadex( NULL, 0, fn, args, CREATE_SUSPENDED, &_id );
  sasASSERT( _threadHandle != NULL );
  resume();
}

void sasThread::suspend()
{
  _active = false;
  SuspendThread( _threadHandle );    
}

void sasThread::resume()
{
  _active = true;
  ResumeThread( _threadHandle );    
}

void sasThread::kill()
{
  TerminateThread( _threadHandle, 1 );
  _active = false;
}

void sasThread::waitForThread()
{
  WaitForSingleObject( _threadHandle, INFINITE ); 
}

