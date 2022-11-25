#pragma once

#include "utils/sas_singleton.h"

struct sasVertexFormat;

class sasDeviceInputLayoutCache : public sasSingleton< sasDeviceInputLayoutCache >
{
  sasNO_COPY( sasDeviceInputLayoutCache );
  sasMEM_OP( sasDeviceInputLayoutCache );

public:
  sasDeviceInputLayoutCache( ID3D11Device* device );
  ~sasDeviceInputLayoutCache();

public:  
  ID3D11InputLayout* getInputLayout( const sasVertexFormat& fmt );

private:
  ID3D11InputLayout* createInputLayout( const sasVertexFormat& fmt );

private:
  typedef std::map< sasVertexFormat, ID3D11InputLayout* > InputLayouts; 

  ID3D11Device* _device;  
  InputLayouts  _inputLayouts;
};