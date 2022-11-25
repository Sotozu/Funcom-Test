#pragma once 

#include "sas_rendertypes.h"

class sasDeviceObject;

struct sasRenderTargetDesc
{
  size_t          width;
  size_t          height;
  size_t          depth;
  sasPixelFormat::Enum  format;
  sasTextureType::Enum  type;
  size_t          mips;
  size_t          needUAV : 1;
  size_t          pad : 31;

  sasRenderTargetDesc()
  {
    width = height = depth = 0;
    format = sasPixelFormat::NONE;
    type = sasTextureType::sasTexture2D;
    mips = 0;
    needUAV = 0;
    pad = 0;
  }
};

class sasRenderTarget
{
  sasNO_COPY( sasRenderTarget );
  sasMEM_OP( sasRenderTarget );  

  friend class sasRenderTargetSetDeviceObject;

public:
  sasRenderTarget( const sasRenderTargetDesc& rtDesc );
  virtual ~sasRenderTarget();

public:
  sasDeviceObject*    deviceObject();  
  size_t              width() const       { return _desc.width; }
  size_t              height() const      { return _desc.height; }
  float               widthFloat() const  { return _widthFloat; }
  float               heightFloat() const { return _heightFloat; }
  size_t              depth() const       { return _desc.depth; }
  size_t              mipCount() const    { return _desc.mips; }
  sasPixelFormat::Enum format() const     { return _desc.format; }
  sasTextureType::Enum type() const       { return _desc.type; }
  bool                hasUAV() const      { return _desc.needUAV; }

private:
  sasRenderTarget() : _deviceObject(NULL) { }

private:
  sasDeviceObject*    _deviceObject;
  sasRenderTargetDesc _desc; 
  float _widthFloat;
  float _heightFloat;
};

class sasRenderTargetSetDeviceObject
{
public:
  sasRenderTargetSetDeviceObject( sasRenderTarget* rt, sasDeviceObject* dob );
};


class sasPingPongRT
{
public:
  sasPingPongRT( sasRenderTarget* ping, sasRenderTarget* pong );
  ~sasPingPongRT() {}

  void swapBuffers()                    { _currentBufferIndex = 1 - _currentBufferIndex; }
  sasRenderTarget* destinationRT()      { return _rts[ _currentBufferIndex ]; }
  sasRenderTarget* sourceRT()           { return _rts[ 1 - _currentBufferIndex ]; }

private:
  sasRenderTarget*  _rts[ 2 ];
  uint32_t          _currentBufferIndex;
};
