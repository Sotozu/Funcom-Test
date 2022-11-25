#pragma once 

#include "sas_rendertypes.h"

class sasDeviceObject;

struct sasDepthStencilTargetDesc
{
  size_t          width;
  size_t          height;
  sasPixelFormat::Enum  format;
  sasTextureType::Enum  type;

  sasDepthStencilTargetDesc()
  {
    width = height = 0;
    format = sasPixelFormat::NONE;
    type = sasTextureType::sasTexture2D;
  }
};

class sasDepthStencilTarget
{
  sasNO_COPY( sasDepthStencilTarget );
  sasMEM_OP( sasDepthStencilTarget );
    
public:
  sasDepthStencilTarget( const sasDepthStencilTargetDesc& dstDesc );
  virtual ~sasDepthStencilTarget();

public:
  sasDeviceObject*    deviceObject();  
  size_t              width() const   { return _desc.width; }
  size_t              height() const  { return _desc.height; }
  sasPixelFormat::Enum format() const { return _desc.format; }
  sasTextureType::Enum type() const   { return _desc.type; }

private:
  sasDepthStencilTarget() : _deviceObject(NULL) { }

private:
  sasDeviceObject*    _deviceObject;
  sasDepthStencilTargetDesc _desc;  
};


