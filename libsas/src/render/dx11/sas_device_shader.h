#pragma once 

#include "sas_device_object.h"
#include "render/sas_rendertypes.h"

class sasDeviceShader : public sasDeviceObject
{
  sasNO_COPY( sasDeviceShader );
  sasMEM_OP( sasDeviceShader );

public:
  sasDeviceShader( sasShaderType::Enum shaderType, void* shader );
  ~sasDeviceShader();

public:
  ID3D11VertexShader*   d3dVertexShader() { sasASSERT( _shaderType == sasShaderType::Vertex );  return _shader.d3dVertexShader; }
  ID3D11PixelShader*    d3dPixelShader()  { sasASSERT( _shaderType == sasShaderType::Pixel );   return _shader.d3dPixelShader; }
  ID3D11ComputeShader*  d3dComputeShader() { sasASSERT( _shaderType == sasShaderType::Compute ); return _shader.d3dComputeShader; }
  ID3D11HullShader*     d3dHullShader() { sasASSERT( _shaderType == sasShaderType::Hull ); return _shader.d3dHullShader; }
  ID3D11DomainShader*   d3dDomainShader() { sasASSERT( _shaderType == sasShaderType::Domain ); return _shader.d3dDomainShader; }
  ID3D11GeometryShader*   d3dGeometryShader() { sasASSERT( _shaderType == sasShaderType::Geometry ); return _shader.d3dGeometryShader; }

private:
#ifdef sasASSERTS_ENABLED
  const sasShaderType::Enum   _shaderType;
#endif
  union 
  {
    void*               d3dPtr;
    ID3D11VertexShader* d3dVertexShader;
    ID3D11HullShader*   d3dHullShader;
    ID3D11DomainShader* d3dDomainShader;
    ID3D11GeometryShader* d3dGeometryShader;
    ID3D11PixelShader*  d3dPixelShader;  
    ID3D11ComputeShader* d3dComputeShader;
  }                     _shader;
};