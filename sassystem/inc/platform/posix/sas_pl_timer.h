#pragma once

#include "utils/sas_singleton.h"

class sasTimer : public sasSingleton< sasTimer >
{
  sasMEM_OP( sasTimer );

public: 
	sasTimer();				
	~sasTimer(){}	
	
	void	update();
	float	getTime()			    { return _time; }			
	float	getElapsedTime()	{ return _elapsedTime; }				
	float	getFPS();
  float	getCachedFPS() const    { return _lastFPS; }

private:
  uint64_t _startTime;
	float 	_time;
	double	_elapsedTime;
  float _lastFPS;
};


