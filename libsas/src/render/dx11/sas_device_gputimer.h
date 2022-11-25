#pragma once

#include "sas_device_object.h"

class sasDeviceGPUTimer : public sasDeviceObject
{
  sasNO_COPY( sasDeviceGPUTimer );
  sasMEM_OP( sasDeviceGPUTimer );

public:
  sasDeviceGPUTimer( ID3D11Query* disjointQuery, ID3D11Query* startQuery, ID3D11Query* endQuery );
  ~sasDeviceGPUTimer();
 
  ID3D11Query*              d3dDisjoingQuery() const { return _disjointQuery; }
  ID3D11Query*              d3dStartQuery() const { return _startQuery; }
  ID3D11Query*              d3dEndQuery() const { return _endQuery; }

private:
  ID3D11Query* _disjointQuery;
  ID3D11Query* _startQuery;
  ID3D11Query* _endQuery;
};
