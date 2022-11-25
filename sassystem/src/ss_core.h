#pragma once 

#include "ss_main.h"

class sasMemoryMng;
class sasScratchMemoryMng;
class sasTimer;

class sasSystemCore
{
  sasNO_COPY(sasSystemCore);

public:
  sasSystemCore();
  ~sasSystemCore();

public:
  void update();

private:

  static const size_t BOOTSTRAP_BUFFER_SIZE = 64;

  char                  _boostrapBuffer[ BOOTSTRAP_BUFFER_SIZE ];
  sasMemoryMng*         _memoryMng; 
  sasScratchMemoryMng*  _scratchMemMng;
  sasTimer*			        _timer;  
};