#pragma once

class sasDeviceObject;

class sasGPUTimer
{
  sasNO_COPY( sasGPUTimer );
  sasMEM_OP( sasGPUTimer );

public:
  sasGPUTimer();
  ~sasGPUTimer();

  sasDeviceObject*  deviceObject();
  
  enum sasGPUTimerState
  {
    sasGPUTimer_started = 0,
    sasGPUTimer_ended,
    sasGPUTimer_resolved,
  };

  sasGPUTimerState state() const { return _timerState; }
  void             setState( sasGPUTimerState state ) { _timerState = state; }
  

private:
  sasDeviceObject*  _deviceObject;
  sasGPUTimerState  _timerState;
 
public:
  float _time;

};


