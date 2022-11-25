#include "sas_pch.h"
#include "sas_timer.h"

#include <mach/mach_time.h>

sasTimer::sasTimer()
{
  _lastFPS = 0.0f;
  _startTime = mach_absolute_time();
  update();
}

void sasTimer::update()
{
  mach_timebase_info_data_t info;
  mach_timebase_info(&info);
  double nano = 1e-9 * ( (double) info.numer) / ((double) info.denom);

  uint64_t currentTime = mach_absolute_time();
  uint64_t elapsedTime = currentTime - _startTime;
  elapsedTime *= info.numer;
  elapsedTime /= info.denom;
  
  _elapsedTime = elapsedTime * 1e-9;
  _time = currentTime * nano;
}

float sasTimer::getFPS()
{
	//Code to obtain FPS every second
	static int iFrames = 0;
	float static fTime, fTimeBase = 0;
	float static fFPS;

	iFrames++;

  fTime = _elapsedTime;
	
	if (fTime - fTimeBase >= 1.f) //if 1 second has passed, calculate the FPS
	{
		fFPS = (iFrames/(fTime-fTimeBase));
		fTimeBase = fTime;		
		iFrames = 0;
	}

  _lastFPS = fFPS;
	return fFPS;
}
