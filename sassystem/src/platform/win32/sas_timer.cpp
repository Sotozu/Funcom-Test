#include "ss_pch.h"
#include "sas_pl_timer.h"
#include "ss_export.h"

ssAPI extern sasTimer* gTimer;

sasTimer::sasTimer()
{
	QueryPerformanceCounter(&_time);
	QueryPerformanceFrequency(&_perfFrequency);
	_elapsedTime = 0.f;
  _lastFPS = 0.f;
  _engineTimeFloat = 0.f;
  _engineTimeDouble = 0.0;
}

void sasTimer::update()
{
	static LARGE_INTEGER currentTime;
	QueryPerformanceCounter( &currentTime );
	_elapsedTime = ( float )( currentTime.QuadPart - _time.QuadPart ) / ( float )_perfFrequency.QuadPart;
	_time = currentTime;
  _engineTimeDouble += static_cast< double >( _elapsedTime );
  _engineTimeFloat = static_cast< float >( _engineTimeDouble );
}

float sasTimer::getExactTime() const
{
  static LARGE_INTEGER currentTime;
  QueryPerformanceCounter( &currentTime );
  return ( float )currentTime.QuadPart / ( float )_perfFrequency.QuadPart;
}

float sasTimer::getFPS()
{
	//Code to obtain FPS every second
	static int iFrames = 0;
	float static fTime, fTimeBase = 0;
	float static fFPS;

	iFrames++;

	fTime = (float)_time.QuadPart / (float)_perfFrequency.QuadPart;
	
	if (fTime - fTimeBase >= 1.f) //if 1 second has passed, calculate the FPS
	{
		fFPS = (iFrames/(fTime-fTimeBase));
		fTimeBase = fTime;		
		iFrames = 0;
	}
  _lastFPS = fFPS;
	return fFPS;
}

sasTimer* sasTimer::singletonPtr()
{
  return gTimer;
}

sasTimer& sasTimer::singleton()
{
  return *gTimer;
}
