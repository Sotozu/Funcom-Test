#include "sas_pch.h"
#include "sas_device_shader.h"
#include "sas_device_common.h"

sasDeviceShader::sasDeviceShader( sasShaderType::Enum shaderType, void* shader )
  : sasDeviceObject( sasDeviceObjectType::Shader )
#ifdef sasASSERTS_ENABLED
  , _shaderType( shaderType )    
#endif
{
  _shader.d3dPtr = shader;
}

sasDeviceShader::~sasDeviceShader()
{
  IUnknown* ptr = (IUnknown*)_shader.d3dPtr;
  DX_RELEASE( ptr );
}