#pragma once 

#include "utils/sas_singleton.h"
#include "render/sas_rendertypes.h"
#include <map>

class sasDeviceObject;
class sasIndexBuffer;
class sasVertexBuffer;
class sasConstantBuffer;
class sasTexture;
class sasRenderTarget;
class sasDepthStencilTarget;
class sasStructuredBuffer;
class sasIndirectArgsBuffer;
class sasGPUTimer;

class sasDeviceFactory : public sasSingleton< sasDeviceFactory >
{
  sasNO_COPY( sasDeviceFactory );  
  sasMEM_OP( sasDeviceFactory );

public:
  sasDeviceFactory( ID3D11Device* device );
  ~sasDeviceFactory();

  sasDeviceObject* createIndexBuffer( const sasIndexBuffer& ib );
  void releaseIndexBuffer( sasDeviceObject* obj );  

  sasDeviceObject* createVertexBuffer( const sasVertexBuffer& vb );
  void releaseVertexBuffer( sasDeviceObject* obj );  

  sasDeviceObject* createShader( const char* file, const char* entryFunc, sasShaderType::Enum shaderType, sasShaderMacro* macros, size_t nMacros );
  void releaseShader( sasDeviceObject* obj );

  sasDeviceObject* createConstantBuffer( const sasConstantBuffer& cb );  
  void releaseConstantBuffer( sasDeviceObject* obj );

  sasDeviceObject* createIndirectArgsBuffer( const sasIndirectArgsBuffer& cb );  
  void releaseIndirectArgsBuffer( sasDeviceObject* obj );

  sasDeviceObject* createStructuredBuffer( const sasStructuredBuffer& cb );  
  void releaseStructuredBuffer( sasDeviceObject* obj );

  sasDeviceObject* createTexture( const sasTexture& tx );
  void releaseTexture( sasDeviceObject* obj );

  sasDeviceObject* createRenderTarget( const sasRenderTarget& tx );
  void releaseRenderTarget( sasDeviceObject* obj );

  sasDeviceObject* createDepthStencilTarget( const sasDepthStencilTarget& tx );
  void releaseDepthStencilTarget( sasDeviceObject* obj );

  sasDeviceObject* createGPUTimer( const sasGPUTimer& gpuTimer );
  void releaseGPUTimer( sasDeviceObject* obj );  

private:
  ID3D11Device*               _device;

};