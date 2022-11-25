#pragma once

#include "ss_export.h"


class sasTimer
{
  sasMEM_OP( sasTimer );

public: 
  sasTimer();				
  ~sasTimer(){}	
  
  ssAPI void	update();
  ssAPI float getExactTime() const;
  ssAPI float	getTime() const		      { return _engineTimeFloat; }	
  ssAPI double getPreciseTime() const { return _engineTimeDouble; }
  ssAPI float	getElapsedTime() const	{ return _elapsedTime; }				
  ssAPI float	getFPS();
  ssAPI float	getCachedFPS() const    { return _lastFPS; }

  ssAPI static sasTimer*  singletonPtr();
  ssAPI static sasTimer&  singleton();

private:
  LARGE_INTEGER	_perfFrequency;
  LARGE_INTEGER	_time;
  float     _engineTimeFloat;
  double    _engineTimeDouble;
  float			_elapsedTime;
  float     _lastFPS;
      
};


