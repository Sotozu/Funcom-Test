#include "sas_pch.h"
#include "sas_device.h"
#include "sas_device_buffer.h"
#include "sas_device_shader.h"
#include "sas_device_common.h"
#include "sas_device_factory.h"
#include "sas_device_inputlayoutcache.h"
#include "sas_device_texture.h"
#include "sas_device_rendertarget.h"
#include "sas_device_gputimer.h"
#include "render/sas_indexbuffer.h"
#include "render/sas_vertexbuffer.h"
#include "render/sas_constantbuffer.h"
#include "render/sas_indirectargsbuffer.h"
#include "render/shaders/sas_shadermng.h"
#include "render/sas_stats.h"
#include "render/sas_texture.h"
#include "render/sas_rendertarget.h"
#include "render/sas_depthbuffer.h"
#include "render/sas_structuredbuffer.h"
#include "render/sas_gputimer.h"
#include "sas_device_statebuffercache.h"
#include "sas_device_depthbuffer.h"
#include "sas_device_typeconversion.h"
#include "utils/sas_color.h"
#include "OVR.h"

sasDevice::sasDevice( const sasConfig& config )
  : _hwnd( (HWND)config.hwnd )
  , _bbWidth(0)
  , _bbHeight(0)
  , _sasDeviceFactory(0)
  , _stateBufferCache(0)
  , _inputLayoutCache(0)
  , _device(0)
  , _deviceContext(0)
  , _swapChain(0)
  , _backBuffer(0)
  , _swapChainDepthBuffer(0)  
  , _flushingQuery(0)
  , _forceGpuFlush( false )
  , _vSyncEnabled( false )
{
  UINT flags = 0;

#ifdef sasDEBUG
  //flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_FEATURE_LEVEL featureLevel;
  DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};  
  HRESULT hr;
  RECT clientRect;

  if( config.vrHmd )
  {
    ovrHmd vrhmd = static_cast< ovrHmd >( config.vrHmd );
    swapChainDesc.BufferDesc.Width = vrhmd->Resolution.w;
    swapChainDesc.BufferDesc.Height = vrhmd->Resolution.h;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
  }
  else
  {
    GetClientRect( _hwnd, &clientRect );
    swapChainDesc.BufferDesc.Width = clientRect.right;
    swapChainDesc.BufferDesc.Height = clientRect.bottom;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
  }
  
  swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
  swapChainDesc.BufferCount = 2;
  swapChainDesc.OutputWindow = _hwnd;
  swapChainDesc.Windowed = config.vrHmd ? FALSE : TRUE;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  hr = D3D11CreateDeviceAndSwapChain( 
    NULL,
    D3D_DRIVER_TYPE_HARDWARE,
    NULL,
    flags,
    NULL,
    0,
    D3D11_SDK_VERSION,
    &swapChainDesc,
    &_swapChain,
    &_device,
    &featureLevel,
    &_deviceContext
    );

  _bbHeight = swapChainDesc.BufferDesc.Height;
  _bbWidth = swapChainDesc.BufferDesc.Width;

  _sasDeviceFactory = sasNew sasDeviceFactory( _device ); 

  bool ok = SUCCEEDED(hr);
  
  ok = ok && initializeBackBuffer( swapChainDesc.BufferDesc.Width, swapChainDesc.BufferDesc.Height );
  ok = ok && initializeDepthBuffer( _backBuffer );

  if (!ok)
  {
    MessageBoxA(0, "Failed to initialize d3d11 device, shutting down...", "sasDevice - initialization", MB_OK|MB_ICONERROR);
    abort();
  }
  
  _inputLayoutCache = sasNew sasDeviceInputLayoutCache( _device ); 
  _stateBufferCache = sasNew sasDeviceStateBufferCache( _device );
  
  setDefaultState();  

  //create flushing query
  D3D11_QUERY_DESC qDesc;
  qDesc.Query = D3D11_QUERY_EVENT;
  qDesc.MiscFlags = 0;
  DX( _device->CreateQuery( &qDesc, &_flushingQuery ) );
}

sasDevice::~sasDevice()
{
  setDefaultState();   

  // reverse order
  sasDelete _stateBufferCache;
  sasDelete _inputLayoutCache;  
  sasDelete _swapChainDepthBuffer;
  sasDelete _backBuffer;    
  sasDelete _sasDeviceFactory;
  DX_RELEASE( _flushingQuery );
  DX_RELEASE( _swapChain );
  DX_RELEASE( _deviceContext );
  DX_RELEASE( _device );  
}

void sasDevice::setDefaultState()
{   
  _changedStateFlags.invalidate();
  _nextState = State();  
  flushStates();  
}

void sasDevice::setIndexBuffer( sasIndexBuffer* obj )
{
  _changedStateFlags.indexBufferChanged = obj != _currentState.indexBuffer;
  _nextState.indexBuffer = obj;
}

void sasDevice::setVertexBuffer( sasVertexBuffer* obj )
{
  _changedStateFlags.vertexBufferChanged = obj != _currentState.vertexBuffer;
  _nextState.vertexBuffer = obj;
}

void sasDevice::setVertexShader( sasShaderID id )
{
  _changedStateFlags.vertexShaderChanged = id != _currentState.vertexShader;
  _nextState.vertexShader = id;
}

void sasDevice::setHullShader( sasShaderID id )
{
  _changedStateFlags.hullShaderChanged = id != _currentState.hullShader;
  _nextState.hullShader = id;
}

void sasDevice::setDomainShader( sasShaderID id )
{
  _changedStateFlags.domainShaderChanged = id != _currentState.domainShader;
  _nextState.domainShader = id;
}

void sasDevice::setGeometryShader( sasShaderID id )
{
  _changedStateFlags.geometryShaderChanged = id != _currentState.geometryShader;
  _nextState.geometryShader = id;
}

void sasDevice::setConstantBuffer( size_t index, sasConstantBuffer* obj, sasDeviceUnit::Enum unit )
{
  if( unit == sasDeviceUnit::VertexShader )
  {
    if ( ( obj != _currentState.constantBufferVS[index] ) || ( _currentState.constantBufferVS[index]->isDirty() ) )
      _changedStateFlags.constantBufferVSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.constantBufferVSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.constantBufferVS[ index ] = obj;
  }
  else if( unit == sasDeviceUnit::HullShader )
  {
    if ( ( obj != _currentState.constantBufferHS[index] ) || ( _currentState.constantBufferHS[index]->isDirty() ) )
      _changedStateFlags.constantBufferHSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.constantBufferHSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.constantBufferHS[ index ] = obj;
  }
  else if( unit == sasDeviceUnit::DomainShader )
  {
    if ( ( obj != _currentState.constantBufferDS[index] ) || ( _currentState.constantBufferDS[index]->isDirty() ) )
      _changedStateFlags.constantBufferDSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.constantBufferDSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.constantBufferDS[ index ] = obj;
  }
  else if( unit == sasDeviceUnit::GeometryShader )
  {
    if ( ( obj != _currentState.constantBufferGS[index] ) || ( _currentState.constantBufferGS[index]->isDirty() ) )
      _changedStateFlags.constantBufferGSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.constantBufferGSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.constantBufferGS[ index ] = obj;
  }
  else if( unit == sasDeviceUnit::PixelShader )
  {
    if ( ( obj != _currentState.constantBufferPS[index] ) || ( _currentState.constantBufferPS[index]->isDirty() ) )
      _changedStateFlags.constantBufferPSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.constantBufferPSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.constantBufferPS[ index ] = obj;
  }
  else if( unit == sasDeviceUnit::ComputeShader )
  {
    if ( ( obj != _currentState.constantBufferCS[index] ) || ( _currentState.constantBufferCS[index]->isDirty() ) )
      _changedStateFlags.constantBufferCSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.constantBufferCSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.constantBufferCS[ index ] = obj;
  }
}

void sasDevice::setPixelShader( sasShaderID id )
{
  _changedStateFlags.pixelShaderChanged = id != _currentState.pixelShader;
  _nextState.pixelShader = id;
}

void sasDevice::setComputeShader( sasShaderID id )
{
  _changedStateFlags.computeShaderChanged = id != _currentState.computeShader;
  _nextState.computeShader = id;
}

void sasDevice::setDepthStencilState( sasDepthStencilState state, unsigned int stencilRef )
{
  _changedStateFlags.depthStencilStateChanged = ( (state != _currentState.depthStencilState) || (stencilRef != _currentState.stencilRef) );
  _nextState.depthStencilState = state;
  _nextState.stencilRef = stencilRef;
}

void sasDevice::setBlendState( sasBlendState state )
{
  _changedStateFlags.blendStateChanged = ( state != _currentState.blendState );
  _nextState.blendState = state;
}

void sasDevice::setRasterizerState( sasRasterizerState state )
{
  _changedStateFlags.rasterizerStateChanged = ( state != _currentState.rasterizerState );
  _nextState.rasterizerState = state;
}

void sasDevice::setSamplerState( size_t index, sasSamplerState sampler, sasDeviceUnit::Enum unit )
{
  if(unit == sasDeviceUnit::PixelShader)
  {
    if ( sampler != _currentState.samplerStatePS[index] )
      _changedStateFlags.samplerStatePSChanged |=  ((uint64_t)1<<index);
    else 
      _changedStateFlags.samplerStatePSChanged &=  ~((uint64_t)1<<index);
    _nextState.samplerStatePS[ index ] = sampler;
  }
  else if(unit == sasDeviceUnit::ComputeShader)
  {
    if ( sampler != _currentState.samplerStateCS[index] )
      _changedStateFlags.samplerStateCSChanged |=  ((uint64_t)1<<index);
    else 
      _changedStateFlags.samplerStateCSChanged &=  ~((uint64_t)1<<index);
    _nextState.samplerStateCS[ index ] = sampler;
  }
  else if(unit == sasDeviceUnit::VertexShader)
  {
    if ( sampler != _currentState.samplerStateVS[index] )
      _changedStateFlags.samplerStateVSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.samplerStateVSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.samplerStateVS[ index ] = sampler;
  }
  else if(unit == sasDeviceUnit::HullShader)
  {
    if ( sampler != _currentState.samplerStateHS[index] )
      _changedStateFlags.samplerStateHSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.samplerStateHSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.samplerStateHS[ index ] = sampler;
  }
  else if(unit == sasDeviceUnit::DomainShader)
  {
    if ( sampler != _currentState.samplerStateDS[index] )
      _changedStateFlags.samplerStateDSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.samplerStateDSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.samplerStateDS[ index ] = sampler;
  }
  else if(unit == sasDeviceUnit::GeometryShader)
  {
    if ( sampler != _currentState.samplerStateGS[index] )
      _changedStateFlags.samplerStateGSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.samplerStateGSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.samplerStateGS[ index ] = sampler;
  }
}


void sasDevice::setTexture( size_t index, sasTexture* obj, sasDeviceUnit::Enum unit )
{
  sasDeviceTexture* tex = obj ? static_cast<sasDeviceTexture*>(obj->deviceObject()) : NULL;
  
  ShaderResource shaderResource;
  shaderResource.srvType = eSRV_TEXTURE;
  shaderResource.resource.texture = tex;

  setTexture( index, &shaderResource, unit );
}

void sasDevice::setTexture( size_t index, sasRenderTarget* obj, sasDeviceUnit::Enum unit )
{
  sasDeviceTexture* tex = obj ? static_cast<sasDeviceRenderTarget*>(obj->deviceObject())->textureObj() : NULL;
  
  ShaderResource shaderResource;
  shaderResource.srvType = eSRV_TEXTURE;
  shaderResource.resource.texture = tex;

  setTexture( index, &shaderResource, unit );
}

void sasDevice::setTexture( size_t index, sasDepthStencilTarget* obj, sasDeviceUnit::Enum unit )
{
  sasDeviceTexture* tex = obj ? static_cast<sasDeviceDepthStencilTarget*>(obj->deviceObject())->textureObj() : NULL;

  ShaderResource shaderResource;
  shaderResource.srvType = eSRV_TEXTURE;
  shaderResource.resource.texture = tex;

  setTexture( index, &shaderResource, unit );
}

void sasDevice::setStructuredBuffer( size_t index, sasStructuredBuffer* obj, sasDeviceUnit::Enum unit )
{
  ShaderResource shaderResource;
  shaderResource.srvType = eSRV_STRUCTURED_BUFFER;
  shaderResource.resource.structuredBuffer = obj;

  setTexture( index, &shaderResource, unit );
}

void sasDevice::setRawBuffer( size_t index, sasIndirectArgsBuffer* obj, sasDeviceUnit::Enum unit )
{
  ShaderResource shaderResource;
  shaderResource.srvType = eSRV_RAW_BUFFER;
  shaderResource.resource.indirectArgsBuffer = obj;

  setTexture( index, &shaderResource, unit );
}

void sasDevice::setTexture( size_t index, ShaderResource* resource, sasDeviceUnit::Enum unit )
{
  if(unit == sasDeviceUnit::PixelShader)
  {
    if ( resource->resource.texture != _currentState.texturePS[index].resource.texture )
      _changedStateFlags.texturePSChanged |=  ((uint64_t)1<<index);
    else 
      _changedStateFlags.texturePSChanged &=  ~((uint64_t)1<<index);
    _nextState.texturePS[ index ] = *resource;
  }
  else if(unit == sasDeviceUnit::HullShader)
  {
    if ( resource->resource.texture != _currentState.textureHS[index].resource.texture )
      _changedStateFlags.textureHSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.textureHSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.textureHS[ index ] = *resource;
  }
  else if(unit == sasDeviceUnit::DomainShader)
  {
    if ( resource->resource.texture != _currentState.textureDS[index].resource.texture )
      _changedStateFlags.textureDSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.textureDSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.textureDS[ index ] = *resource;
  }
  else if(unit == sasDeviceUnit::GeometryShader)
  {
    if ( resource->resource.texture != _currentState.textureGS[index].resource.texture )
      _changedStateFlags.textureGSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.textureGSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.textureGS[ index ] = *resource;
  }
  else if(unit == sasDeviceUnit::VertexShader)
  {
    if ( resource->resource.texture != _currentState.textureVS[index].resource.texture )
      _changedStateFlags.textureVSChanged |=  ((unsigned __int64)1<<index);
    else 
      _changedStateFlags.textureVSChanged &=  ~((unsigned __int64)1<<index);
    _nextState.textureVS[ index ] = *resource;
  }
  else if(unit == sasDeviceUnit::ComputeShader)
  {
    if ( resource->resource.texture != _currentState.textureCS[index].resource.texture )
      _changedStateFlags.textureCSChanged |=  ((uint64_t)1<<index);
    else 
      _changedStateFlags.textureCSChanged &=  ~((uint64_t)1<<index);
    _nextState.textureCS[ index ] = *resource;
  }
}

void sasDevice::setRenderTarget( size_t index, sasRenderTarget* rt )
{
  if ( rt != _currentState.renderTargets[index] )
    _changedStateFlags.renderTargetChanged |=  ((uint64_t)1<<index);
  else 
    _changedStateFlags.renderTargetChanged &=  ~((uint64_t)1<<index);
  _nextState.renderTargets[ index ] = rt;
}

void sasDevice::setUAV( size_t index, sasRenderTarget* uav, sasDeviceUnit::Enum unit, unsigned int initCount )
{
  sasASSERT( uav ? uav->hasUAV() : true );
  sasASSERT( (unit == sasDeviceUnit::ComputeShader) || (unit == sasDeviceUnit::PixelShader) );
  
  UAVResource uavRT;
  uavRT.isRenderTarget = true;
  uavRT.resource.renderTarget = uav;
  uavRT.initialCount = initCount;
  setUAV( index, &uavRT, unit );
}

void sasDevice::setUAV( size_t index, sasStructuredBuffer* uav, sasDeviceUnit::Enum unit, unsigned int initCount )
{
  sasASSERT( uav ? !uav->dynamic() : true );
  sasASSERT( (unit == sasDeviceUnit::ComputeShader) || (unit == sasDeviceUnit::PixelShader) );

  UAVResource uavSB;
  uavSB.isRenderTarget = false;
  uavSB.resource.structuredBuffer = uav;
  uavSB.initialCount = initCount;
  setUAV( index, &uavSB, unit );
}

void sasDevice::setUAV( size_t index, UAVResource* uav, sasDeviceUnit::Enum unit )
{
  if( uav->resource.renderTarget != NULL )
  {
    if( uav->isRenderTarget )
    {
      sasASSERT( uav->resource.renderTarget->hasUAV() );
    }
    else
    {
      sasASSERT( !uav->resource.structuredBuffer->dynamic() );
    }
  }
  sasASSERT( (unit == sasDeviceUnit::ComputeShader) || (unit == sasDeviceUnit::PixelShader) );

  if( unit == sasDeviceUnit::PixelShader )
  {
    if ( uav->resource.renderTarget != _currentState.uavs[index].resource.renderTarget )
      _changedStateFlags.uavsChanged |=  ((uint64_t)1<<index);
    else 
      _changedStateFlags.uavsChanged &=  ~((uint64_t)1<<index);
    _nextState.uavs[ index ] = *uav;
  }
  else if( unit == sasDeviceUnit::ComputeShader )
  {
    if ( ( uav->resource.renderTarget != _currentState.computeUavs[index].resource.renderTarget ) ||
          ( uav->initialCount != _currentState.computeUavs[ index ].initialCount ) )
      _changedStateFlags.computeUavsChanged |=  ((uint64_t)1<<index);
    else 
      _changedStateFlags.computeUavsChanged &=  ~((uint64_t)1<<index);
    _nextState.computeUavs[ index ] = *uav;
  }
}

void sasDevice::setViewport( const sasViewport& vp )
{
  _changedStateFlags.viewportChanged = vp != _currentState.viewport;
  if( _changedStateFlags.viewportChanged )
  {
    _nextState.viewport = vp;
  }  
}

void sasDevice::setViewport( sasRenderTarget* rt )
{
  sasViewport vp;
  vp.topLeftX = vp.topLeftY = 0;
  vp.minDepth = 0; vp.maxDepth = 1.f;
  vp.width = static_cast< uint16_t >( rt->width() );
  vp.height = static_cast< uint16_t >( rt->height() );
  setViewport( vp );
}

void sasDevice::setViewport( sasDepthStencilTarget* dst )
{
  sasViewport vp;
  vp.topLeftX = vp.topLeftY = 0;
  vp.minDepth = 0; vp.maxDepth = 1.f;
  vp.width = static_cast< uint16_t >( dst->width() );
  vp.height = static_cast< uint16_t >( dst->height() );
  setViewport( vp );
}

void sasDevice::setDepthStencilTarget(sasDepthStencilTarget* dst, bool readOnly)
{
  sasDepthStencilTarget* depthBuffer = dst ? static_cast<sasDepthStencilTarget*>(dst) : NULL;
  
  if ( (depthBuffer != _currentState.depthStencilTarget) | (_currentState.readOnlyDepthStencilTarget != readOnly) )
    _changedStateFlags.depthStencilTargetChanged |=  ((uint64_t)1);
  else 
    _changedStateFlags.depthStencilTargetChanged &=  ~((uint64_t)1);

  _nextState.depthStencilTarget = depthBuffer;
  _nextState.readOnlyDepthStencilTarget = readOnly;
}

void sasDevice::draw( sasPrimTopology primTopology, size_t nVerts )
{   
  _changedStateFlags.primTopologyChanged = ( primTopology != _currentState.primTopology );
  _nextState.primTopology = primTopology;

  flushStates();

  _deviceContext->Draw( nVerts, 0 );
 
  sasRenderStats::singleton().incDrawCount();  
}

void sasDevice::drawInstancedIndirect( sasPrimTopology primTopology, sasIndirectArgsBuffer* argsBuffer )
{
  _changedStateFlags.primTopologyChanged = ( primTopology != _currentState.primTopology );
  _nextState.primTopology = primTopology;

  flushStates();

  _deviceContext->DrawInstancedIndirect( static_cast< sasDeviceBuffer* >( argsBuffer->deviceObject() )->d3dBuffer(), 0 );

  sasRenderStats::singleton().incDrawCount(); 
}

void sasDevice::drawIndexed( sasPrimTopology primTopology, size_t nPrims )
{
  _changedStateFlags.primTopologyChanged = ( primTopology != _currentState.primTopology );
  _nextState.primTopology = primTopology;

  flushStates();

  _deviceContext->DrawIndexed( nPrims*3, 0, 0 );

  sasRenderStats::singleton().incDrawCount();  
}

void sasDevice::drawIndexedInstanced( sasPrimTopology primTopology, size_t nPrims, size_t instanceCount )
{
  _changedStateFlags.primTopologyChanged = ( primTopology != _currentState.primTopology );
  _nextState.primTopology = primTopology;

  flushStates();

  _deviceContext->DrawIndexedInstanced( nPrims * 3, instanceCount, 0, 0, 0 );

  sasRenderStats::singleton().incDrawCount();
}

void sasDevice::dispatch( size_t threadGroupCountX, size_t threadGroupCountY, size_t threadGroupCountZ )
{
  flushStates();
  _deviceContext->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
  sasRenderStats::singleton().incDrawCount();  
}

void sasDevice::flushStates( )
{
  size_t nStateChanges = 0;

  // --------------------
  //  Topology
  // --------------------
  if( _changedStateFlags.primTopologyChanged )
  {
    _deviceContext->IASetPrimitiveTopology( sasGetDevicePrimTopology(_nextState.primTopology) );
  }

  // --------------------
  //  Index Buffer
  // --------------------

  if( _changedStateFlags.indexBufferChanged )
  {
    if( _nextState.indexBuffer )
    {
      sasDeviceBuffer* indexBuffer = (sasDeviceBuffer*)_nextState.indexBuffer->deviceObject();
      _deviceContext->IASetIndexBuffer( indexBuffer->d3dBuffer(), DXGI_FORMAT_R16_UINT, 0 );
    }    
    else
    {
      _deviceContext->IASetIndexBuffer( 0, DXGI_FORMAT_R16_UINT, 0 );
    }  
    ++nStateChanges;
  }

  // --------------------
  //  Vertex Buffer
  // --------------------
  if( _changedStateFlags.vertexBufferChanged )
  {  
    if( _nextState.vertexBuffer )
    {
      sasDeviceBuffer* vertexBuffer = (sasDeviceBuffer*)_nextState.vertexBuffer->deviceObject();
      ID3D11Buffer* d3dVertexBuffers = vertexBuffer->d3dBuffer();
      UINT strides = _nextState.vertexBuffer->format().size();
      UINT offsets = 0;  
      ID3D11InputLayout* ia = sasDeviceInputLayoutCache::singleton().getInputLayout( _nextState.vertexBuffer->format() );  
      _deviceContext->IASetInputLayout( ia );
      _deviceContext->IASetVertexBuffers( 0, 1, &d3dVertexBuffers, &strides, &offsets ); 
    }
    else
    {
      _deviceContext->IASetInputLayout( 0 );
      _deviceContext->IASetVertexBuffers( 0, 0, 0, 0, 0 ); 
    }   
    ++nStateChanges;
  }

  // --------------------
  //  Vertex Shader
  // --------------------

  if( _changedStateFlags.vertexShaderChanged )
  {
    if( _nextState.vertexShader.isValid() )
    {
      sasDeviceShader* vertexShader = (sasDeviceShader*)sasShaderMng::singleton().getVertexShader( _nextState.vertexShader );
      if( vertexShader != NULL )
      {
        ID3D11VertexShader* d3dVertexShader = vertexShader->d3dVertexShader();
        _deviceContext->VSSetShader( d3dVertexShader, 0, 0 );      
      }
      else
      {
        _deviceContext->VSSetShader( 0, 0, 0 );
      }
    }
    else
    {
      _deviceContext->VSSetShader( 0, 0, 0 );
    }    
    ++nStateChanges;
  }

  // --------------------
  //  Vertex Shader 
  //  Constant Buffer
  // --------------------

  if( _changedStateFlags.constantBufferVSChanged )
  {
    ID3D11Buffer* d3dConstantBuffers[6] = { 0 };
    for( size_t i = 0; i < 6; i++ )
    {
      if( _nextState.constantBufferVS[i] )
      {
        sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.constantBufferVS[i]->deviceObject());
        updateConstantBuffer( _nextState.constantBufferVS[i] );
        d3dConstantBuffers[i] = deviceBuffer->d3dBuffer();
      }      
    }
    _deviceContext->VSSetConstantBuffers(0, 6, d3dConstantBuffers);

    ++nStateChanges;
  }

  // --------------------
  //  Hull Shader
  // --------------------

  if( _changedStateFlags.hullShaderChanged )
  {
    if( _nextState.hullShader.isValid() )
    {
      sasDeviceShader* hullShader = (sasDeviceShader*)sasShaderMng::singleton().getHullShader( _nextState.hullShader );
      ID3D11HullShader* d3dHullShader = hullShader->d3dHullShader();
      _deviceContext->HSSetShader( d3dHullShader, 0, 0 );      
    }
    else
    {
      _deviceContext->HSSetShader( 0, 0, 0 );
    }    
    ++nStateChanges;
  }

  // --------------------
  //  Hull Shader 
  //  Constant Buffer
  // --------------------

  if( _changedStateFlags.constantBufferHSChanged )
  {
    ID3D11Buffer* d3dConstantBuffers[6] = { 0 };
    for( size_t i = 0; i < 6; i++ )
    {
      if( _nextState.constantBufferHS[i] )
      {
        sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.constantBufferHS[i]->deviceObject());
        updateConstantBuffer( _nextState.constantBufferHS[i] );
        d3dConstantBuffers[i] = deviceBuffer->d3dBuffer();
      }      
    }
    _deviceContext->HSSetConstantBuffers(0, 6, d3dConstantBuffers);

    ++nStateChanges;
  }

  // --------------------
  //  Domain Shader
  // --------------------

  if( _changedStateFlags.domainShaderChanged )
  {
    if( _nextState.domainShader.isValid() )
    {
      sasDeviceShader* domainShader = (sasDeviceShader*)sasShaderMng::singleton().getDomainShader( _nextState.domainShader );
      ID3D11DomainShader* d3dDomainShader = domainShader->d3dDomainShader();
      _deviceContext->DSSetShader( d3dDomainShader, 0, 0 );      
    }
    else
    {
      _deviceContext->DSSetShader( 0, 0, 0 );
    }    
    ++nStateChanges;
  }

  // --------------------
  //  Domain Shader 
  //  Constant Buffer
  // --------------------

  if( _changedStateFlags.constantBufferDSChanged )
  {
    ID3D11Buffer* d3dConstantBuffers[6] = { 0 };
    for( size_t i = 0; i < 6; i++ )
    {
      if( _nextState.constantBufferDS[i] )
      {
        sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.constantBufferDS[i]->deviceObject());
        updateConstantBuffer( _nextState.constantBufferDS[i] );
        d3dConstantBuffers[i] = deviceBuffer->d3dBuffer();
      }      
    }
    _deviceContext->DSSetConstantBuffers(0, 6, d3dConstantBuffers);

    ++nStateChanges;
  }

  // --------------------
  //  Geometry Shader
  // --------------------

  if( _changedStateFlags.geometryShaderChanged )
  {
    if( _nextState.geometryShader.isValid() )
    {
      sasDeviceShader* geometryShader = (sasDeviceShader*)sasShaderMng::singleton().getGeometryShader( _nextState.geometryShader );
      ID3D11GeometryShader* d3dGeometryShader = geometryShader->d3dGeometryShader();
      _deviceContext->GSSetShader( d3dGeometryShader, 0, 0 );      
    }
    else
    {
      _deviceContext->GSSetShader( 0, 0, 0 );
    }    
    ++nStateChanges;
  }

  // --------------------
  //  Geometry Shader 
  //  Constant Buffer
  // --------------------

  if( _changedStateFlags.constantBufferGSChanged )
  {
    ID3D11Buffer* d3dConstantBuffers[6] = { 0 };
    for( size_t i = 0; i < 6; i++ )
    {
      if( _nextState.constantBufferGS[i] )
      {
        sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.constantBufferGS[i]->deviceObject());
        updateConstantBuffer( _nextState.constantBufferGS[i] );
        d3dConstantBuffers[i] = deviceBuffer->d3dBuffer();
      }      
    }
    _deviceContext->GSSetConstantBuffers(0, 6, d3dConstantBuffers);

    ++nStateChanges;
  }

  // --------------------
  //  Pixel Shader 
  //  Constant Buffer
  // --------------------
  if( _changedStateFlags.constantBufferPSChanged )
  {
    ID3D11Buffer* d3dConstantBuffers[6] = { 0 };
    for( size_t i = 0; i < 6; i++ )
    {
      if( _nextState.constantBufferPS[i] )
      {
        sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.constantBufferPS[i]->deviceObject());
        updateConstantBuffer( _nextState.constantBufferPS[i] );
        d3dConstantBuffers[i] = deviceBuffer->d3dBuffer();
      }      
    }
    _deviceContext->PSSetConstantBuffers(0, 6, d3dConstantBuffers);

    ++nStateChanges;
  }

  // --------------------
  //  Compute Shader 
  //  Constant Buffer
  // --------------------

  if( _changedStateFlags.constantBufferCSChanged )
  {
    ID3D11Buffer* d3dConstantBuffers[6] = { 0 };
    for( size_t i = 0; i < 6; i++ )
    {
      if( _nextState.constantBufferCS[i] )
      {
        sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.constantBufferCS[i]->deviceObject());
        updateConstantBuffer( _nextState.constantBufferCS[i] );
        d3dConstantBuffers[i] = deviceBuffer->d3dBuffer();
      }      
    }
    _deviceContext->CSSetConstantBuffers(0, 6, d3dConstantBuffers);

    ++nStateChanges;
  }

  // --------------------
  //  Pixel Shader
  // --------------------

  if( _changedStateFlags.pixelShaderChanged )
  {
    if( _nextState.pixelShader.isValid() )
    {
      sasDeviceShader* pixelShader = (sasDeviceShader*)sasShaderMng::singleton().getPixelShader( _nextState.pixelShader );
      if( pixelShader != NULL )
      {
        ID3D11PixelShader* d3dPixelShader = pixelShader->d3dPixelShader();  
        _deviceContext->PSSetShader( d3dPixelShader, 0, 0 );
      }
      else
      {
        _deviceContext->PSSetShader( 0, 0, 0 );
      }
    }
    else
    {
      _deviceContext->PSSetShader( 0, 0, 0 );
    }  
    ++nStateChanges;
  }

  // --------------------
  //  ComputeShader
  // --------------------

  if( _changedStateFlags.computeShaderChanged )
  {
    if( _nextState.computeShader.isValid() )
    {
      sasDeviceShader* computeShader = (sasDeviceShader*)sasShaderMng::singleton().getComputeShader( _nextState.computeShader );  
      ID3D11ComputeShader* d3dComputeShader = computeShader->d3dComputeShader();  
      _deviceContext->CSSetShader( d3dComputeShader, 0, 0 );
    }
    else
    {
      _deviceContext->CSSetShader( 0, 0, 0 );
    }  
    ++nStateChanges;
  }

  // --------------------
  //  Blend states
  // --------------------

  if( _changedStateFlags.blendStateChanged )
  {
    if( _nextState.blendState != sasBlendState_Null )
    {
      ID3D11BlendState* d3dState = _stateBufferCache->requestBlendState(_nextState.blendState);
      _deviceContext->OMSetBlendState( d3dState, 0, 0xffffffff);
    }
    else
    {
      _deviceContext->OMSetBlendState( NULL, 0, 0xffffffff);
    }   
    ++nStateChanges;
  }

  // --------------------
  //  Rasterizer states
  // --------------------

  if( _changedStateFlags.rasterizerStateChanged )
  {
    if( _nextState.rasterizerState != sasRasterizerState_Null )
    {
      ID3D11RasterizerState* d3dState = _stateBufferCache->requestRasterizerState(_nextState.rasterizerState);
      _deviceContext->RSSetState( d3dState );
    }
    else
    {
      _deviceContext->RSSetState( NULL );
    }   
    ++nStateChanges;
  }  

  // --------------------
  //  Render targets
  // --------------------

  if( _changedStateFlags.renderTargetChanged | _changedStateFlags.uavsChanged | _changedStateFlags.depthStencilTargetChanged )
  {
    unsigned int numRTVs = 0;
    ID3D11RenderTargetView* rtViews[4] = { 0 };
    for( size_t i=0; i<4; i++ )
    {
      if( _nextState.renderTargets[i] )
      {              
        rtViews[i] = static_cast<sasDeviceRenderTarget*>(_nextState.renderTargets[i]->deviceObject())->d3dRTV();
        ++numRTVs;
      }            
    }

    ID3D11UnorderedAccessView* uaViews[4] = { 0 };
    for( size_t i=0; i<4; i++ )
    {
      if( _nextState.uavs[i].resource.renderTarget )
      {
        if(_nextState.uavs[i].isRenderTarget)
          uaViews[i] = static_cast<sasDeviceRenderTarget*>(_nextState.uavs[i].resource.renderTarget->deviceObject())->d3dUAV();
        else
          uaViews[i] = static_cast<sasDeviceBuffer*>(_nextState.uavs[i].resource.structuredBuffer->deviceObject())->d3dUAV();
      }            
    }

    ID3D11DepthStencilView* dsView = NULL;
    if( _nextState.depthStencilTarget )
    {
      if(_nextState.readOnlyDepthStencilTarget)
        dsView = static_cast<sasDeviceDepthStencilTarget*>(_nextState.depthStencilTarget->deviceObject())->d3dReadOnlyDSV();
      else
        dsView =  static_cast<sasDeviceDepthStencilTarget*>(_nextState.depthStencilTarget->deviceObject())->d3dDSV();
    }

    _deviceContext->OMSetRenderTargetsAndUnorderedAccessViews( 4, rtViews, dsView, 4, 4, uaViews, NULL );
  }

  //compute shader uavs
  if( _changedStateFlags.computeUavsChanged )
  {
    ID3D11UnorderedAccessView* uaViews[4] = { 0, 0, 0, 0 };
    UINT initCounts[ 4 ] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    for( size_t i=0; i<4; i++ )
    {
      if( _nextState.computeUavs[i].resource.renderTarget )
      {              
        if(_nextState.computeUavs[i].isRenderTarget)
          uaViews[i] = static_cast<sasDeviceRenderTarget*>(_nextState.computeUavs[i].resource.renderTarget->deviceObject())->d3dUAV();
        else
          uaViews[i] = static_cast<sasDeviceBuffer*>(_nextState.computeUavs[i].resource.structuredBuffer->deviceObject())->d3dUAV();

        initCounts[ i ] = _nextState.computeUavs[ i ].initialCount;
      }  
    }

    _deviceContext->CSSetUnorderedAccessViews( 0, 4, uaViews, &initCounts[ 0 ] );
  }

  // --------------------
  //  Viewport
  // --------------------

  if( _changedStateFlags.viewportChanged )
  {
    D3D11_VIEWPORT dxVp;
    sasGetDeviceViewport( _nextState.viewport, &dxVp);
    _deviceContext->RSSetViewports(1, &dxVp);
  }

  // --------------------
  //  Depth Stencil states
  // --------------------

  if( _changedStateFlags.depthStencilStateChanged )
  {
    if( _nextState.depthStencilState != sasDepthStencilState_Null )
    {
      ID3D11DepthStencilState* d3dState = _stateBufferCache->requestDepthStencilState(_nextState.depthStencilState); 
      _deviceContext->OMSetDepthStencilState( d3dState, _nextState.stencilRef );
    }
    else
    {
      _deviceContext->OMSetDepthStencilState( NULL, 0 );
    }   
    ++nStateChanges;
  }

  // --------------------
  //  Sampler PS
  // --------------------

  if( _changedStateFlags.samplerStatePSChanged )
  {
    ID3D11SamplerState* samplerStates[8] = { 0 };
    for( size_t i=0; i<8; i++ )
    {
      if( _nextState.samplerStatePS[i] != sasSamplerState_Null )
      {
        samplerStates[i] = _stateBufferCache->requestSamplerState(_nextState.samplerStatePS[i]);          
      }    
    }
    _deviceContext->PSSetSamplers( 0, 8, samplerStates );
  }

  // --------------------
  //  Texture PS
  // --------------------

  if( _changedStateFlags.texturePSChanged )
  {
    ID3D11ShaderResourceView* textureViews[8] = { 0 };
    for( size_t i=0; i<8; i++ )
    {
      if( _nextState.texturePS[i].resource.texture )
      {              
        if( _nextState.texturePS[i].srvType == eSRV_TEXTURE )
          textureViews[i] = _nextState.texturePS[i].resource.texture->d3dShaderResourceView();
        else if( _nextState.texturePS[i].srvType == eSRV_STRUCTURED_BUFFER )
        {
          sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.texturePS[i].resource.structuredBuffer->deviceObject());
          updateStructuredBuffer( _nextState.texturePS[i].resource.structuredBuffer );
          textureViews[i] = deviceBuffer->d3dSRV();
        }
        else if( _nextState.texturePS[i].srvType == eSRV_RAW_BUFFER )
        {
          sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.texturePS[i].resource.indirectArgsBuffer->deviceObject());
          textureViews[i] = deviceBuffer->d3dSRV();
        }
      }            
    }
    _deviceContext->PSSetShaderResources( 0, 8, textureViews );
  }

  // --------------------
  //  Sampler CS
  // --------------------

  if( _changedStateFlags.samplerStateCSChanged )
  {
    ID3D11SamplerState* samplerStates[8] = { 0 };
    for( size_t i=0; i<8; i++ )
    {
      if( _nextState.samplerStateCS[i] != sasSamplerState_Null )
      {
        samplerStates[i] = _stateBufferCache->requestSamplerState(_nextState.samplerStateCS[i]);          
      }    
    }
    _deviceContext->CSSetSamplers( 0, 8, samplerStates );
  }

  // --------------------
  //  Texture CS
  // --------------------

  if( _changedStateFlags.textureCSChanged )
  {
    ID3D11ShaderResourceView* textureViews[ 16 ] = { 0 };
    for( size_t i = 0; i < 16; i++ )
    {
      if( _nextState.textureCS[ i ].resource.texture )
      {           
        if( _nextState.textureCS[ i ].srvType == eSRV_TEXTURE )
          textureViews[ i ] = _nextState.textureCS[ i ].resource.texture->d3dShaderResourceView();
        else if( _nextState.textureCS[ i ].srvType == eSRV_STRUCTURED_BUFFER )
        {
          sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>( _nextState.textureCS[i].resource.structuredBuffer->deviceObject() );
          updateStructuredBuffer( _nextState.textureCS[ i ].resource.structuredBuffer );
          textureViews[ i ] = deviceBuffer->d3dSRV();
        }
        else if( _nextState.textureCS[ i ].srvType == eSRV_RAW_BUFFER )
        {
          sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>( _nextState.textureCS[i].resource.indirectArgsBuffer->deviceObject() );
          textureViews[ i ] = deviceBuffer->d3dSRV();
        }
      }            
    }
    _deviceContext->CSSetShaderResources( 0, 16, textureViews );
  }

  // --------------------
  //  Texture VS
  // --------------------

  if( _changedStateFlags.textureVSChanged )
  {
    ID3D11ShaderResourceView* textureViews[4] = { 0 };
    for( size_t i=0; i<4; i++ )
    {
      if( _nextState.textureVS[i].resource.texture )
      {              
        if( _nextState.textureVS[i].srvType == eSRV_TEXTURE )
          textureViews[i] = _nextState.textureVS[i].resource.texture->d3dShaderResourceView();
        else if( _nextState.textureVS[i].srvType == eSRV_STRUCTURED_BUFFER )
        {
          sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.textureVS[i].resource.structuredBuffer->deviceObject());
          updateStructuredBuffer( _nextState.textureVS[i].resource.structuredBuffer );
          textureViews[i] = deviceBuffer->d3dSRV();
        }
        else if( _nextState.textureVS[i].srvType == eSRV_RAW_BUFFER )
        {
          sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.textureVS[i].resource.indirectArgsBuffer->deviceObject());
          textureViews[i] = deviceBuffer->d3dSRV();
        }
      }            
    }
    _deviceContext->VSSetShaderResources( 0, 4, textureViews );
  }

  // --------------------
  //  Sampler VS
  // --------------------

  if( _changedStateFlags.samplerStateVSChanged )
  {
    ID3D11SamplerState* samplerStates[4] = { 0 };
    for( size_t i=0; i<4; i++ )
    {
      if( _nextState.samplerStateVS[i] != sasSamplerState_Null )
      {
        samplerStates[i] = _stateBufferCache->requestSamplerState(_nextState.samplerStateVS[i]);          
      }    
    }
    _deviceContext->VSSetSamplers( 0, 4, samplerStates );
  }

  // --------------------
  //  Sampler HS
  // --------------------

  if( _changedStateFlags.samplerStateHSChanged )
  {
    ID3D11SamplerState* samplerStates[8] = { 0 };
    for( size_t i=0; i<8; i++ )
    {
      if( _nextState.samplerStateHS[i] != sasSamplerState_Null )
      {
        samplerStates[i] = _stateBufferCache->requestSamplerState(_nextState.samplerStateHS[i]);          
      }    
    }
    _deviceContext->HSSetSamplers( 0, 8, samplerStates );
  }

  // --------------------
  //  Texture HS
  // --------------------

  if( _changedStateFlags.textureHSChanged )
  {
    ID3D11ShaderResourceView* textureViews[8] = { 0 };
    for( size_t i=0; i<8; i++ )
    {
      if( _nextState.textureHS[i].resource.texture )
      {              
        if( _nextState.textureHS[i].srvType == eSRV_TEXTURE )
          textureViews[i] = _nextState.textureHS[i].resource.texture->d3dShaderResourceView();
        else if( _nextState.textureHS[i].srvType == eSRV_STRUCTURED_BUFFER )
        {
          sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.textureHS[i].resource.structuredBuffer->deviceObject());
          updateStructuredBuffer( _nextState.textureHS[i].resource.structuredBuffer );
          textureViews[i] = deviceBuffer->d3dSRV();
        }
        else if( _nextState.textureHS[i].srvType == eSRV_RAW_BUFFER )
        {
          sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.textureHS[i].resource.indirectArgsBuffer->deviceObject());
          textureViews[i] = deviceBuffer->d3dSRV();
        }
      }            
    }
    _deviceContext->HSSetShaderResources( 0, 8, textureViews );
  }

  // --------------------
  //  Sampler DS
  // --------------------

  if( _changedStateFlags.samplerStateDSChanged )
  {
    ID3D11SamplerState* samplerStates[8] = { 0 };
    for( size_t i=0; i<8; i++ )
    {
      if( _nextState.samplerStateDS[i] != sasSamplerState_Null )
      {
        samplerStates[i] = _stateBufferCache->requestSamplerState(_nextState.samplerStateDS[i]);          
      }    
    }
    _deviceContext->DSSetSamplers( 0, 8, samplerStates );
  }

  // --------------------
  //  Texture DS
  // --------------------

  if( _changedStateFlags.textureDSChanged )
  {
    ID3D11ShaderResourceView* textureViews[8] = { 0 };
    for( size_t i=0; i<8; i++ )
    {
      if( _nextState.textureDS[i].resource.texture )
      {              
        if( _nextState.textureDS[i].srvType == eSRV_TEXTURE )
          textureViews[i] = _nextState.textureDS[i].resource.texture->d3dShaderResourceView();
        else if( _nextState.textureDS[i].srvType == eSRV_STRUCTURED_BUFFER )
        {
          sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.textureDS[i].resource.structuredBuffer->deviceObject());
          updateStructuredBuffer( _nextState.textureDS[i].resource.structuredBuffer );
          textureViews[i] = deviceBuffer->d3dSRV();
        }
        else if( _nextState.textureDS[i].srvType == eSRV_RAW_BUFFER )
        {
          sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.textureDS[i].resource.indirectArgsBuffer->deviceObject());
          textureViews[i] = deviceBuffer->d3dSRV();
        }
      }            
    }
    _deviceContext->DSSetShaderResources( 0, 8, textureViews );
  }

  // --------------------
  //  Sampler GS
  // --------------------

  if( _changedStateFlags.samplerStateGSChanged )
  {
    ID3D11SamplerState* samplerStates[3] = { 0 };
    for( size_t i=0; i<3; i++ )
    {
      if( _nextState.samplerStateGS[i] != sasSamplerState_Null )
      {
        samplerStates[i] = _stateBufferCache->requestSamplerState(_nextState.samplerStateGS[i]);          
      }    
    }
    _deviceContext->GSSetSamplers( 0, 3, samplerStates );
  }

  // --------------------
  //  Texture GS
  // --------------------

  if( _changedStateFlags.textureGSChanged )
  {
    ID3D11ShaderResourceView* textureViews[3] = { 0 };
    for( size_t i=0; i<3; i++ )
    {
      if( _nextState.textureGS[i].resource.texture )
      {              
        if( _nextState.textureGS[i].srvType == eSRV_TEXTURE )
          textureViews[i] = _nextState.textureGS[i].resource.texture->d3dShaderResourceView();
        else if( _nextState.textureGS[i].srvType == eSRV_STRUCTURED_BUFFER )
        {
          sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.textureGS[i].resource.structuredBuffer->deviceObject());
          updateStructuredBuffer( _nextState.textureGS[i].resource.structuredBuffer );
          textureViews[i] = deviceBuffer->d3dSRV();
        }
        else if( _nextState.textureGS[i].srvType == eSRV_RAW_BUFFER )
        {
          sasDeviceBuffer* deviceBuffer = static_cast<sasDeviceBuffer*>(_nextState.textureGS[i].resource.indirectArgsBuffer->deviceObject());
          textureViews[i] = deviceBuffer->d3dSRV();
        }
      }            
    }
    _deviceContext->GSSetShaderResources( 0, 3, textureViews );
  }

  // --------------------
  //  Finalize
  // -------------------- 

  if( sasRenderStats::isValid() )
    sasRenderStats::singleton().incStateChanges( nStateChanges );
  _currentState = _nextState;
  _changedStateFlags.validate();

  
}


bool sasDevice::initializeBackBuffer( size_t width, size_t height )
{
  sasASSERT( !_backBuffer );

  HRESULT hr;
  sasRenderTargetDesc bbDesc;
  bbDesc.format = sasPixelFormat::RGBA_U8;
  bbDesc.width = width;
  bbDesc.height = height;
  bbDesc.mips = 1;
  bbDesc.needUAV = false;
  _backBuffer = sasNew sasRenderTarget( bbDesc ); 

  ID3D11Texture2D* dxBackBufferTexture = NULL;
  ID3D11ShaderResourceView* dxBackBufferSRV = NULL;
  ID3D11RenderTargetView* dxBackBufferRTV = NULL;

  DX( _swapChain->GetBuffer( 0, __uuidof(dxBackBufferTexture), (void**)&dxBackBufferTexture ) );

  if SUCCEEDED(hr)
  {
    D3D11_RENDER_TARGET_VIEW_DESC rtd;
    rtd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtd.Texture2D.MipSlice = 0;

    DX( _device->CreateRenderTargetView( dxBackBufferTexture, &rtd, &dxBackBufferRTV) );
    DX( _device->CreateShaderResourceView( dxBackBufferTexture, NULL, &dxBackBufferSRV) );

    sasDeviceRenderTarget* renderTarget = sasNew sasDeviceRenderTarget( dxBackBufferTexture, NULL, dxBackBufferSRV, dxBackBufferRTV, NULL );

    sasRenderTargetSetDeviceObject( _backBuffer, renderTarget );
  }

  return SUCCEEDED( hr );
}

void sasDevice::shutdownBackBuffer()
{
  if(_backBuffer)
    sasDelete _backBuffer;
  _backBuffer = 0;
}

bool sasDevice::initializeDepthBuffer( sasRenderTarget* rt )
{
  sasASSERT( !_swapChainDepthBuffer );

  sasDepthStencilTargetDesc dstDesc;
  dstDesc.format = sasPixelFormat::D24S8;  
  dstDesc.width = rt->width();
  dstDesc.height = rt->height();
  _swapChainDepthBuffer = sasNew sasDepthStencilTarget( dstDesc);
  return 0 != _swapChainDepthBuffer->deviceObject();
}

void sasDevice::shutdownDepthBuffer()
{
  if(_swapChainDepthBuffer)
    sasDelete _swapChainDepthBuffer;
  _swapChainDepthBuffer = 0;
}

bool sasDevice::resizeBackBuffer()
{
  HRESULT hr;

  RECT rect;
  GetClientRect( (HWND)_hwnd, &rect );
  unsigned int width = rect.right;
  unsigned int height = rect.bottom;

  if( false ) //if( _bbWidth != width || _bbHeight != height )
  {
    _deviceContext->OMSetRenderTargets( 0, 0, 0 );
    shutdownDepthBuffer();
    shutdownBackBuffer();    

    DX( _swapChain->ResizeBuffers( 2, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) );

    initializeBackBuffer( width, height );
    initializeDepthBuffer( _backBuffer );

    _bbWidth = width;
    _bbHeight = height;

    for( size_t i = 0; i < _resetFuncs.size(); i++ )
    {
      _resetFuncs[ i ]();
    }

    return true;
  }

  return false;
}

IDXGIOutput* sasDevice::findDisplayOutput( const std::string& displayName )
{
  IDXGIOutput* desiredOutput = NULL;

  if( displayName.length() > 0 )
  {
    IDXGIFactory* dxgiFactory = NULL;
    HRESULT hr = CreateDXGIFactory( __uuidof(IDXGIFactory), (void**)( &dxgiFactory ) );
    if( FAILED( hr ) )    
      return NULL;

    for(UINT AdapterIndex = 0; ; AdapterIndex++)
    {
      IDXGIAdapter* adapter;

      hr = dxgiFactory->EnumAdapters( AdapterIndex, &adapter );
      if (hr == DXGI_ERROR_NOT_FOUND)
        break;

      DXGI_ADAPTER_DESC Desc;
      adapter->GetDesc(&Desc);

      for(UINT OutputIndex = 0; ; OutputIndex++)
      {
        IDXGIOutput* output;
        hr = adapter->EnumOutputs(OutputIndex, &output);
        if( hr == DXGI_ERROR_NOT_FOUND )
        {
          break;
        }

        DXGI_OUTPUT_DESC OutDesc;
        output->GetDesc( &OutDesc );

        MONITORINFOEX monitor;
        monitor.cbSize = sizeof( monitor );
        if( ::GetMonitorInfo( OutDesc.Monitor, &monitor ) && monitor.szDevice[ 0 ] )
        {
          DISPLAY_DEVICE dispDev;
          memset( &dispDev, 0, sizeof( dispDev ) );
          dispDev.cb = sizeof( dispDev );

          if( ::EnumDisplayDevices( monitor.szDevice, 0, &dispDev, 0 ) )
          {
            if( strstr( std::string( dispDev.DeviceName ).c_str(), displayName.c_str() ) )
            {
              desiredOutput = output;
              break;
            }
          }
        }
      }
      if( desiredOutput != NULL )
        break;
    }

    DX_RELEASE( dxgiFactory );
  }

  return desiredOutput;
}

bool sasDevice::setFullscreenMode( size_t width, size_t height, const std::string& displayName, bool state )
{
  HRESULT hr;
  //resize BB
  _deviceContext->OMSetRenderTargets( 0, 0, 0 );
  shutdownDepthBuffer();
  shutdownBackBuffer();    

  DX( _swapChain->ResizeBuffers( 2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) );

  initializeBackBuffer( width, height );
  initializeDepthBuffer( _backBuffer );

  _bbWidth = width;
  _bbHeight = height;

  //set fullscreen mode
  IDXGIOutput* fullscreenOutput = findDisplayOutput( displayName );
  DX( _swapChain->SetFullscreenState( state ? TRUE : FALSE, state ? fullscreenOutput : NULL ) );
  if( FAILED( hr ) )
    return false;

  for( size_t i = 0; i < _resetFuncs.size(); i++ )
  {
    _resetFuncs[ i ]();
  }

  return true;
}

void sasDevice::copyStructureCount( sasConstantBuffer *dstBuffer, unsigned int offset, sasStructuredBuffer* srcBuffer )
{
  _deviceContext->CopyStructureCount( static_cast< sasDeviceBuffer* >( dstBuffer->deviceObject() )->d3dBuffer(), 
                                          offset, 
                                          static_cast< sasDeviceBuffer* >( srcBuffer->deviceObject() )->d3dUAV() );
}

void sasDevice::copyStructureCount( sasIndirectArgsBuffer *dstBuffer, unsigned int offset, sasStructuredBuffer* srcBuffer )
{
  _deviceContext->CopyStructureCount( static_cast< sasDeviceBuffer* >( dstBuffer->deviceObject() )->d3dBuffer(), 
    offset, 
    static_cast< sasDeviceBuffer* >( srcBuffer->deviceObject() )->d3dUAV() );
}

void sasDevice::copyResource( sasStructuredBuffer* dstBuffer, sasStructuredBuffer* srcBuffer )
{
  _deviceContext->CopyResource( static_cast< sasDeviceBuffer* >( dstBuffer->deviceObject() )->d3dBuffer(), 
                                static_cast< sasDeviceBuffer* >( srcBuffer->deviceObject() )->d3dBuffer() );
}

void sasDevice::presentFrame()
{
  HRESULT hr;
  if( _vSyncEnabled )
    DX( _swapChain->Present( 1, 0 ) );
  else
    DX( _swapChain->Present( 0, 0 ) );

  if( _forceGpuFlush )
  {
    _deviceContext->End( _flushingQuery );
    BOOL flushed = FALSE;
    do { }
    while( !flushed && !FAILED( _deviceContext->GetData( _flushingQuery, &flushed, sizeof( flushed ), 0 ) ) );
  }
}

void sasDevice::clearRenderTarget( sasRenderTarget* rt, const sasColor* clearColour )
{
  flushStates();
  _deviceContext->ClearRenderTargetView( static_cast<sasDeviceRenderTarget*>(rt->deviceObject())->d3dRTV(), *clearColour );
}

void sasDevice::clearDepthStencilTarget( sasDepthStencilTarget* dst, unsigned int flags, float depthValue, UINT8 stencilValue )
{
  flushStates();
  _deviceContext->ClearDepthStencilView( static_cast<sasDeviceDepthStencilTarget*>(dst->deviceObject())->d3dDSV(), sasGetDeviceClearFlags(flags), depthValue, stencilValue );
}

void sasDevice::updateConstantBuffer( sasConstantBuffer* obj ) const
{
  if( obj->isDirty() )
  {
    ID3D11Buffer* buffer = static_cast<sasDeviceBuffer*>(obj->deviceObject())->d3dBuffer();
    D3D11_MAPPED_SUBRESOURCE mappedSubResource = { 0 };
    HRESULT hr;
    DX( _deviceContext->Map( buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource ) );
    if( SUCCEEDED( hr ) )
    {
      memcpy( mappedSubResource.pData, obj->buffer(), obj->size() );
      _deviceContext->Unmap( buffer, 0 );
    }
    obj->validate();
  }
}

void sasDevice::updateStructuredBuffer( sasStructuredBuffer* obj ) const
{
  if( obj->isDirty() )
  {
    ID3D11Buffer* buffer = static_cast<sasDeviceBuffer*>(obj->deviceObject())->d3dBuffer();
    D3D11_MAPPED_SUBRESOURCE mappedSubResource = { 0 };
    HRESULT hr;
    DX( _deviceContext->Map( buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource ) );
    if( SUCCEEDED( hr ) )
    {
      memcpy( mappedSubResource.pData, obj->buffer(), obj->size() );
      _deviceContext->Unmap( buffer, 0 );
    }
    obj->validate();
  }
}

void sasDevice::startTimer( sasGPUTimer& gpuTimer )
{
  sasDeviceGPUTimer* deviceObj = static_cast< sasDeviceGPUTimer* >( gpuTimer.deviceObject() );
  sasASSERT( gpuTimer.state() == sasGPUTimer::sasGPUTimer_resolved );

  _deviceContext->Begin( deviceObj->d3dDisjoingQuery() );
  _deviceContext->End( deviceObj->d3dStartQuery() );

  gpuTimer.setState( sasGPUTimer::sasGPUTimer_started );
}

void sasDevice::endTimer( sasGPUTimer& gpuTimer )
{
  sasDeviceGPUTimer* deviceObj = static_cast< sasDeviceGPUTimer* >( gpuTimer.deviceObject() );
  sasASSERT( gpuTimer.state() == sasGPUTimer::sasGPUTimer_started );

  _deviceContext->End( deviceObj->d3dEndQuery() );
  _deviceContext->End( deviceObj->d3dDisjoingQuery() );

  gpuTimer.setState( sasGPUTimer::sasGPUTimer_ended );
}

float sasDevice::resolveTimer( sasGPUTimer& gpuTimer, bool spinUntilReady )
{
  sasASSERT( spinUntilReady ); //latent queries unimplemented

  sasDeviceGPUTimer* deviceObj = static_cast< sasDeviceGPUTimer* >( gpuTimer.deviceObject() );
  sasASSERT( gpuTimer.state() == sasGPUTimer::sasGPUTimer_ended );

  // Get the query data
  UINT64 startTime = 0;
  while( _deviceContext->GetData( deviceObj->d3dStartQuery(), &startTime, sizeof( startTime ), 0 ) != S_OK );

  UINT64 endTime = 0;
  while( _deviceContext->GetData( deviceObj->d3dEndQuery(), &endTime, sizeof( endTime ), 0) != S_OK );

  D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
  while( _deviceContext->GetData( deviceObj->d3dDisjoingQuery(), &disjointData, sizeof( disjointData ), 0) != S_OK );

  float time = 0.0f;
  if( disjointData.Disjoint == FALSE )
  {
    UINT64 delta = endTime - startTime;
    float frequency = static_cast< float >( disjointData.Frequency );
    time = ( delta / frequency ) * 1000.0f;
  }        

  gpuTimer.setState( sasGPUTimer::sasGPUTimer_resolved );
  gpuTimer._time = time;
  return time;
}
