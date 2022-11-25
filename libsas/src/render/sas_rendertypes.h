#pragma once 

// ----------------------------------------------------------------------------
// sasShaderType
// ----------------------------------------------------------------------------

class sasShaderType
{
public:
  enum Enum 
  {
    Vertex,
    Hull,
    Domain,
    Geometry,
    Pixel,
    Compute,
  };
};

// ----------------------------------------------------------------------------
// sasShaderID
// ----------------------------------------------------------------------------

struct sasShaderID
{
  static const uint64_t INVALID = ~0;

  enum {      
    NBITS_SHADERFLAGS = 48,
    NBITS_FAMILY      = 8,    
   };

  sasShaderID() 
    : flags( INVALID ) {}

  union 
  {
    struct 
    {
      uint64_t shaderFlags  : NBITS_SHADERFLAGS;
      uint64_t family       : NBITS_FAMILY;
    };
    uint64_t flags;
  };  

  bool isValid() const { return INVALID!=flags; }  
};

inline bool operator!=( sasShaderID lhs, sasShaderID rhs )
{
  return lhs.flags != rhs.flags;
}

inline bool operator==( sasShaderID lhs, sasShaderID rhs )
{
  return lhs.flags == rhs.flags;
}

inline bool operator<( sasShaderID lhs, sasShaderID rhs )
{
  return lhs.flags < rhs.flags;
}

inline sasShaderID operator|( sasShaderID lhs, sasShaderID rhs )
{
  sasShaderID r;
  if( rhs.isValid() )
    r.flags = lhs.flags | rhs.flags;
  else
    r.flags = lhs.flags;
  return r;
}

// ----------------------------------------------------------------------------
// sasShaderID
// ----------------------------------------------------------------------------

struct sasShaderMacro
{
  enum { VALUE_STRING_LEN = 32 - sizeof(char*) };
  const char* define;
  char        value[  VALUE_STRING_LEN ];
};
sasSTATIC_ASSERT( sasALIGNED16( sizeof(sasShaderMacro) ) );


// ----------------------------------------------------------------------------
// state block types
// ----------------------------------------------------------------------------

enum sasSamplerState
{
  sasSamplerState_Null = 0, 
  sasSamplerState_Bilinear_Wrap,  
  sasSamplerState_Bilinear_Clamp,
  sasSamplerState_Linear_ShadowCompare,
  sasSamplerState_Point_Clamp,
  sasSamplerState_Point_Wrap,
  sasSamplerState_Point_Border,
  sasSamplerState_Point_ShadowCompare,
  sasSamplerState_Anisotropic_Clamp,
  sasSamplerState_Anisotropic_Wrap,

  sasSamplerStateCount,
};

enum sasDepthStencilState
{
  sasDepthStencilState_Null = 0,
  sasDepthStencilState_LessEqual,
  sasDepthStencilState_NoZWrite_LessEqual,
  sasDepthStencilState_NoZWrite_Less,
  sasDepthStencilState_NoZWrite_Greater,
  sasDepthStencilState_NoZWrite_Equal,
  sasDepthStencilState_NoZWrite_NoZTest,              // No Z write and no Z testing
  sasDepthStencilState_NoZTest,                       // Z write, but no Z testing

  sasDepthStencilStateCount,
};

enum sasBlendState
{
  // These are particle system specific blend states, they will set additional
  // state such as seperateAlphaBlend, etc...
  //
  sasBlendState_Null = 0,
  sasBlendState_Opaque,
  sasBlendState_Opaque_NoColourWrites,
  sasBlendState_One_One,
  sasBlendState_AlphaInvAlpha,

  sasBlendStateCount,
};

enum sasRasterizerState
{
  // These are particle system specific blend states, they will set additional
  // state such as seperateAlphaBlend, etc...
  //
  sasRasterizerState_Null = 0,
  sasRasterizerState_Solid,
  sasRasterizerState_Wireframe,
  sasRasterizerState_Solid_NoCull,
  sasRasterizerState_Wireframe_NoCull,

  sasRasterizerStateCount,
  
};

enum sasCapability
{
  sasCapability_ComputeShaders = 0,
  sasCapability_Tesselation,
  sasCapability_VolumetricLights,
  sasCapability_MotionBlur
};

// ----------------------------------------------------------------------------
// sasPixelFormat
// ----------------------------------------------------------------------------

class sasPixelFormat 
{
public:
  enum Enum 
  {
    NONE,
    BGRA_U8,
    DXT1,
    DXT5,
    RGBA_U8,
    RGBA_F16,
    RGBA_F32,
    R_F16,
    R_F32,
    RG_F16,
    D24S8,
    D32,
    D16,
  };
};

class sasTextureType
{
public:
  enum Enum
  {
    sasTexture2D,
    sasTexture3D,
    sasTextureCube,
  };
};

enum sasClearFlag
{
  sasClearFlag_Depth          = 1,
  sasClearFlag_Stencil        = 2,
  sasClearFlag_DepthStencil = sasClearFlag_Depth|sasClearFlag_Stencil
};

enum sasPrimTopology
{
  sasPrimTopology_TriList = 0,
  sasPrimTopology_TriStrip,
  sasPrimTopology_TriPatch,
  sasPrimTopology_PointList,
  sasPrimTopology_LineList,
};

// ----------------------------------------------------------------------------
// sasDeviceUnit
// ----------------------------------------------------------------------------

class sasDeviceUnit
{
public:
  enum Enum 
  {
    VertexShader,
    HullShader,
    DomainShader,
    GeometryShader,
    PixelShader,
    ComputeShader,
  };
};

// ----------------------------------------------------------------------------
// sasMaterialType
// ----------------------------------------------------------------------------
namespace sasMaterialType
{
  enum Enum
  {
    Opaque = 0,
    AlphaTested,
    AlphaBlended,
    Additive,
  };
}


// ----------------------------------------------------------------------------
// sasViewport
// ----------------------------------------------------------------------------

struct sasViewport
{
  uint16_t topLeftX;
  uint16_t topLeftY;
  uint16_t width;
  uint16_t height;
  float minDepth;
  float maxDepth; 

  sasViewport()
    : topLeftX(0)
    , topLeftY(0)
    , width(0)
    , height(0)
    , minDepth(0)
    , maxDepth(1.f)
  { 
  }

  bool operator==( const sasViewport& other ) const 
  {
    return 
      topLeftX == other.topLeftX &&
      topLeftY == other.topLeftY &&
      width == other.width &&
      height == other.height &&
      minDepth == other.minDepth &&
      maxDepth == other.maxDepth;          
  } 

  bool operator!=( const sasViewport& other ) const 
  {
    return !(*this == other );
  }  
};


// ----------------------------------------------------------------------------
// sasPerInstanceConstants
// ----------------------------------------------------------------------------

struct sasPerInstanceConstants
{
  smMat44 _worldMtx;
  smMat44 _lastFrameWorldMtx;
  smVec4  _instanceId; //yzw unused
  smVec4  _tintColour;
};

// ----------------------------------------------------------------------------
// ResourceMng types
// ----------------------------------------------------------------------------
typedef uint64_t sasTextureId;

// ----------------------------------------------------------------------------
// device management
// ----------------------------------------------------------------------------
typedef bool ( *sasDeviceResetFn )( void );


/*
  shaders
*/
struct sasShaderBlob
{
  sasShaderID   _shaderID;
  uint32_t      _stageMask;
};
