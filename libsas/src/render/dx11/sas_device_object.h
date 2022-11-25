#pragma once 

class sasDeviceObjectType
{
public:
  enum Enum 
  {
    Buffer,
    Shader,
    SamplerState,
    BlendState,
    RasterizerState, 
    DepthStencilState,
    Texture,
    RenderTarget,
    GPUTimer,
  };
};

class sasDeviceObject 
{
  sasNO_COPY( sasDeviceObject );

protected:
  sasDeviceObject( sasDeviceObjectType::Enum deviceObjectType )
    : _deviceObjectType( deviceObjectType )
  {}
  
#ifdef sasDEBUG
  virtual ~sasDeviceObject() {}
#endif

public:
  bool isA( sasDeviceObjectType::Enum t ) const { return t == _deviceObjectType; }

private:
  const sasDeviceObjectType::Enum _deviceObjectType;
};
