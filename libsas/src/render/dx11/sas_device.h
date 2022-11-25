#pragma once 

#include "utils/sas_singleton.h"
#include "render/sas_rendertypes.h"

class sasDeviceFactory;
class sasDeviceInputLayoutCache;
class sasIndexBuffer;
class sasVertexBuffer;
class sasConstantBuffer;
class sasIndirectArgsBuffer;
class sasTexture;
class sasRenderTarget;
class sasDeviceTexture;
class sasDeviceStateBufferCache;
struct sasColor;
class sasDepthStencilTarget;
class sasStructuredBuffer;
class sasDeviceBuffer;
class sasGPUTimer;
class sasStereo;

class sasDevice : public sasSingleton< sasDevice >
{
  sasNO_COPY( sasDevice );
  sasMEM_OP( sasDevice );  

  friend class sasStereo;
public: //! ctor/dtor
   
  sasDevice( const sasConfig& config );
  ~sasDevice();

public: //! accessors

  size_t                  backBufferWidth() const  { return _bbWidth; }
  size_t                  backBufferHeight() const { return _bbHeight; }

  ID3D11Device*           d3dDevice() const         { return _device; }
  ID3D11DeviceContext*    d3dDeviceContext() const  {return _deviceContext; }

  sasRenderTarget*        backBuffer() const { return _backBuffer; }
  sasDepthStencilTarget*  depthBuffer() const { return _swapChainDepthBuffer; }  

public: //! methods

  void setDefaultState();
  
  void setIndexBuffer( sasIndexBuffer* obj );
  void setVertexBuffer( sasVertexBuffer* obj );

  void setVertexShader( sasShaderID id );
  void setHullShader( sasShaderID id );
  void setDomainShader( sasShaderID id );
  void setGeometryShader( sasShaderID id );
  void setPixelShader( sasShaderID id );
  void setComputeShader( sasShaderID id );

  void setConstantBuffer( size_t index, sasConstantBuffer* obj, sasDeviceUnit::Enum unit );  
  void setUAV( size_t index, sasRenderTarget* uav, sasDeviceUnit::Enum unit, unsigned int initCount = -1 );  
  void setUAV( size_t index, sasStructuredBuffer* uav, sasDeviceUnit::Enum unit, unsigned int initCount = -1 );  
  void setStructuredBuffer( size_t index, sasStructuredBuffer* obj, sasDeviceUnit::Enum unit );
  void setRawBuffer( size_t index, sasIndirectArgsBuffer* obj, sasDeviceUnit::Enum unit );
  
  void setDepthStencilState( sasDepthStencilState state, unsigned int stencilRef );
  void setBlendState( sasBlendState state );
  void setRasterizerState( sasRasterizerState state );
  void setSamplerState( size_t index, sasSamplerState sampler, sasDeviceUnit::Enum unit );

  void setTexture( size_t index, sasTexture* obj, sasDeviceUnit::Enum unit );
  void setTexture( size_t index, sasRenderTarget* obj, sasDeviceUnit::Enum unit );
  void setTexture( size_t index, sasDepthStencilTarget* obj, sasDeviceUnit::Enum unit );
  
  void setRenderTarget( size_t index, sasRenderTarget* rt);
  void setDepthStencilTarget( sasDepthStencilTarget* dst, bool readOnly );  
    
  void setViewport( const sasViewport& vp );
  void setViewport( sasRenderTarget* rt );
  void setViewport( sasDepthStencilTarget* dst );
  
  void clearRenderTarget( sasRenderTarget* rt, const sasColor* clearColour );
  void clearDepthStencilTarget( sasDepthStencilTarget* dst, unsigned int flags, float depthValue, UINT8 stencilValue );
  void draw( sasPrimTopology primTopology, size_t nVerts );
  void drawIndexed( sasPrimTopology primTopology, size_t nPrims );
  void drawIndexedInstanced( sasPrimTopology primTopology, size_t nPrims, size_t instanceCount );
  void drawInstancedIndirect( sasPrimTopology primTopology, sasIndirectArgsBuffer* argsBuffer );
  void dispatch( size_t threadGroupCountX, size_t threadGroupCountY, size_t threadGroupCountZ );
  
  void copyStructureCount( sasConstantBuffer *dstBuffer, unsigned int offset, sasStructuredBuffer* srcBuffer );
  void copyStructureCount( sasIndirectArgsBuffer *dstBuffer, unsigned int offset, sasStructuredBuffer* srcBuffer );
  void copyResource( sasStructuredBuffer* dstBuffer, sasStructuredBuffer* srcBuffer );
  void presentFrame();

  bool resizeBackBuffer( ); 
  bool setFullscreenMode( size_t width, size_t height, const std::string& displayName, bool state );
  void flushStates( );
  
  bool queryCapability( sasCapability capability ) { return true; }
  void reloadShaders() {}

  void startTimer( sasGPUTimer& gpuTimer );
  void endTimer( sasGPUTimer& gpuTimer );
  float resolveTimer( sasGPUTimer& gpuTimer, bool spinUntilReady = true );

  void setVSyncEnabled( bool state )    { _vSyncEnabled = state; }
  void setForceGPUFlush( bool state )   { _forceGpuFlush = state; }

  void registerDeviceResetCallback( sasDeviceResetFn fn ) { _resetFuncs.push_back( fn ); }

private: //! types
  enum eShaderResourceType
  {
    eSRV_RAW_BUFFER,
    eSRV_STRUCTURED_BUFFER,
    eSRV_TEXTURE,
  };
  struct ShaderResource 
  {
    union 
    {
      sasDeviceTexture* texture;
      sasStructuredBuffer* structuredBuffer;
      sasIndirectArgsBuffer* indirectArgsBuffer;
    } resource;
    eShaderResourceType srvType;
  };

  struct UAVResource 
  {
    union 
    {
      sasRenderTarget* renderTarget;
      sasStructuredBuffer* structuredBuffer;
    } resource;
    unsigned int initialCount;
    bool isRenderTarget;
  };

private: //! inner methods

  bool initializeBackBuffer( size_t width, size_t height );
  void shutdownBackBuffer();
  bool initializeDepthBuffer( sasRenderTarget* rt );
  void shutdownDepthBuffer();
  void setTexture( size_t index, ShaderResource* resource, sasDeviceUnit::Enum unit );
  void setUAV( size_t index, UAVResource* resource, sasDeviceUnit::Enum unit );
  void updateConstantBuffer( sasConstantBuffer* obj ) const;
  void updateStructuredBuffer( sasStructuredBuffer* obj ) const;
  IDXGIOutput* findDisplayOutput( const std::string& displayName );

private: //! members

  struct State 
  {
    State()
    {
      // 
      // This is the default state of the gpu.      
      // We assume that setting the default state put the gpu in a clean state
      // State must be explicitly set before a draw call as we can't rely on
      // a default arbitrary state.
      // Ex:
      // device->setDefaultState( )
      // device->setBlendState( xxx )
      // device->draw( xxx )
      //
      indexBuffer = 0;
      vertexBuffer = 0;      
      memset(constantBufferVS, NULL, sizeof(constantBufferVS));
      memset(constantBufferHS, NULL, sizeof(constantBufferHS));
      memset(constantBufferDS, NULL, sizeof(constantBufferDS));
      memset(constantBufferGS, NULL, sizeof(constantBufferGS));
      memset(constantBufferPS, NULL, sizeof(constantBufferPS));
      memset(constantBufferCS, NULL, sizeof(constantBufferCS));
      vertexShader = sasShaderID();
      hullShader = sasShaderID();
      domainShader = sasShaderID();
      geometryShader = sasShaderID();
      pixelShader = sasShaderID();
      computeShader = sasShaderID();
      blendState = sasBlendState_Null;
      rasterizerState = sasRasterizerState_Null;
      depthStencilState = sasDepthStencilState_Null;
      stencilRef = 0;
      memset( samplerStatePS, (int)sasSamplerState_Null, sizeof( samplerStatePS ) );
      memset( samplerStateHS, (int)sasSamplerState_Null, sizeof( samplerStateHS ) );
      memset( samplerStateDS, (int)sasSamplerState_Null, sizeof( samplerStateDS ) );
      memset( samplerStateGS, (int)sasSamplerState_Null, sizeof( samplerStateGS ) );
      memset( samplerStateVS, (int)sasSamplerState_Null, sizeof( samplerStateVS ) );
      memset( texturePS, 0, sizeof( texturePS ) );
      memset( samplerStateCS, (int)sasSamplerState_Null, sizeof( samplerStateCS) );      
      memset( textureCS, 0, sizeof( textureCS ) );
      memset( textureVS, 0, sizeof( textureVS ) );
      memset( textureHS, 0, sizeof( textureHS ) );
      memset( textureDS, 0, sizeof( textureDS ) );
      memset( textureGS, 0, sizeof( textureGS ) );
      memset( renderTargets, 0, sizeof( renderTargets ) );
      memset( uavs, 0, sizeof( uavs ) );
      memset( computeUavs, 0, sizeof( computeUavs ) );
      depthStencilTarget = 0;
      readOnlyDepthStencilTarget = false;
      primTopology = sasPrimTopology_TriList;
    }
    sasIndexBuffer*             indexBuffer;
    sasVertexBuffer*            vertexBuffer;
    sasConstantBuffer*          constantBufferVS[ 6 ];
    sasConstantBuffer*          constantBufferHS[ 6 ];
    sasConstantBuffer*          constantBufferDS[ 6 ];
    sasConstantBuffer*          constantBufferGS[ 6 ];
    sasConstantBuffer*          constantBufferPS[ 6 ];
    sasConstantBuffer*          constantBufferCS[ 6 ];
    sasShaderID                 vertexShader;
    sasShaderID                 hullShader;
    sasShaderID                 domainShader;
    sasShaderID                 geometryShader;
    sasShaderID                 pixelShader; 
    sasShaderID                 computeShader;
    sasBlendState               blendState;
    sasRasterizerState          rasterizerState;
    sasDepthStencilState        depthStencilState;
    unsigned int                stencilRef;
    sasSamplerState             samplerStatePS[ 8 ];
    ShaderResource              texturePS[ 16 ];
    sasSamplerState             samplerStateCS[ 8 ];
    ShaderResource              textureCS[ 16 ];
    sasRenderTarget*            renderTargets[ 4 ];
    UAVResource                 uavs[ 4 ];
    UAVResource                 computeUavs[ 4 ];
    sasDepthStencilTarget*      depthStencilTarget;
    size_t                      readOnlyDepthStencilTarget : 1;
    sasPrimTopology             primTopology;
    sasViewport                 viewport;        
    ShaderResource              textureVS[ 4 ];
    sasSamplerState             samplerStateVS[ 4 ];
    ShaderResource              textureHS[ 8 ];
    sasSamplerState             samplerStateHS[ 8 ];
    ShaderResource              textureDS[ 8 ];
    sasSamplerState             samplerStateDS[ 8 ];
    ShaderResource              textureGS[ 3 ];
    sasSamplerState             samplerStateGS[ 3 ];
  };

  union ChangedStateFlags
  {    
    ChangedStateFlags() 
      : flags1( 0 )
      , flags2( 0 )
      , flags3( 0 )
    {}

    void invalidate()
    {
      flags1 = ~0;
      flags2 = ~0;
      flags3 = ~0;
    }

    void validate()
    {
      flags1 = 0;
      flags2 = 0;
      flags3 = 0;
    }

    struct 
    {
      // -------------------------------------------------- index of the first bit
      uint64_t      indexBufferChanged        : 1;  // 0
      uint64_t      vertexBufferChanged       : 1;  // 1
      uint64_t      constantBufferVSChanged   : 6;  // 2
      uint64_t      vertexShaderChanged       : 1;  // 8
      uint64_t      pixelShaderChanged        : 1;  // 9
      uint64_t      computeShaderChanged      : 1;  // 10
      uint64_t      blendStateChanged         : 1;  // 11
      uint64_t      rasterizerStateChanged    : 1;  // 12
      uint64_t      depthStencilStateChanged  : 1;  // 13
      uint64_t      samplerStatePSChanged     : 8;  // 14
      uint64_t      texturePSChanged          : 16; // 22
      uint64_t      renderTargetChanged       : 4;  // 38
      uint64_t      uavsChanged               : 4;  // 42
      uint64_t      depthStencilTargetChanged : 1;  // 46
      uint64_t      primTopologyChanged       : 1;  // 47
      uint64_t      samplerStateCSChanged     : 8;  // 48
      uint64_t      samplerStateDSChanged     : 8;  // 56

      uint64_t      viewportChanged           : 1;  // 0
      uint64_t      constantBufferPSChanged   : 6;  // 1
      uint64_t      constantBufferCSChanged   : 6;  // 7
      uint64_t      domainShaderChanged       : 1;  // 13
      uint64_t      textureVSChanged          : 4;  // 14
      uint64_t      samplerStateVSChanged     : 4;  // 18
      uint64_t      computeUavsChanged        : 4;  // 22
      uint64_t      hullShaderChanged         : 1;  // 26
      uint64_t      constantBufferHSChanged   : 6;  // 27
      uint64_t      constantBufferDSChanged   : 6;  // 33
      uint64_t      textureHSChanged          : 8;  // 39
      uint64_t      samplerStateHSChanged     : 8;  // 47
      uint64_t      textureDSChanged          : 8;  // 55
      uint64_t      geometryShaderChanged     : 1;  // 63

      uint64_t      textureCSChanged          : 16;  // 0
      uint64_t      constantBufferGSChanged   : 6;  // 16
      uint64_t      textureGSChanged          : 3;  // 22
      uint64_t      samplerStateGSChanged     : 3;  // 25 ->28
      
    };
    struct 
    {
      uint64_t      flags1;
      uint64_t      flags2;
      uint64_t      flags3;
    };
  };
  

  HWND                        _hwnd;
  size_t			                _bbWidth;
  size_t			                _bbHeight;
  sasDeviceFactory*           _sasDeviceFactory;
  sasDeviceStateBufferCache*  _stateBufferCache;
  sasDeviceInputLayoutCache*  _inputLayoutCache;
  ID3D11Device*               _device;
  ID3D11DeviceContext*        _deviceContext;
  IDXGISwapChain*             _swapChain;
  sasRenderTarget*            _backBuffer;
  sasDepthStencilTarget*      _swapChainDepthBuffer;  
  ID3D11Query*                _flushingQuery;

  std::vector< sasDeviceResetFn > _resetFuncs;

  ChangedStateFlags           _changedStateFlags;  
  State                       _currentState;
  State                       _nextState;
  bool                        _vSyncEnabled;
  bool                        _forceGpuFlush;
};
