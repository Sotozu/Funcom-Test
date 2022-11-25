#pragma once

#include "ss_export.h"
#include <windows.h>

#define sasStdCall __stdcall

typedef unsigned int sasThreadId;
typedef unsigned int ( sasStdCall *sasThreadFn )( void* );

class ssAPI sasThread
{
  sasMEM_OP( sasThread );
public:
  sasThread();
  ~sasThread();

  void start( sasThreadFn fn, void* args );
  void suspend();
  void resume();
  void kill();
  void waitForThread();

  bool        isActive() const { return _active; }
  sasThreadId threadId() const { return _id; }

private:
  HANDLE        _threadHandle;
  sasThreadId   _id;
  bool          _active;

};
