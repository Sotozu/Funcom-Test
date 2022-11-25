#include "sas_pch.h"
#include "sas_lightpass.h"
#include "sas_light.h"
#include "sas_lightmng.h"
#include "sas_device.h"
#include "../shaders/sas_shaderids.h"
#include "../shaders/sas_shadermng.h"
#include "../sas_rendertarget.h"
#include "../sas_depthbuffer.h"
#include "../sas_constantbuffer.h"
#include "../sas_structuredbuffer.h"
#include "../sas_stats.h"
#include "utils/sas_frustum.h"

struct lightingConstantBuffer
{
  smVec4 sunDirection;
  smVec4 sunColour;
  int numPointLights;
  int numSpotLights;
};

sasLightPass::sasLightPass( )
{
  _constantBuffer = sasNew sasConstantBuffer( sizeof( lightingConstantBuffer ) );
}

sasLightPass::~sasLightPass( )
{
  sasDelete _constantBuffer;
}

void sasLightPass::updateLightData( const sasFrustum& frustum )
{
  sasLightMng* lightMng = sasLightMng::singletonPtr();

  lightMng->cullLights( frustum );

  lightingConstantBuffer * constantBuffer = static_cast< lightingConstantBuffer* >( _constantBuffer->map() );
  constantBuffer->sunColour = lightMng->sunColour();
  constantBuffer->sunDirection = lightMng->sunDir();
  constantBuffer->numPointLights = lightMng->culledPointLightArray().size(); 
  constantBuffer->numSpotLights = lightMng->culledSpotLightArray().size();
  smNormalize3( constantBuffer->sunDirection, constantBuffer->sunDirection );
  _constantBuffer->unmap();

  sasRenderStats::singleton().setPointLightCount( lightMng->culledPointLightArray().size() );
  sasRenderStats::singleton().setSpotLightCount( lightMng->culledSpotLightArray().size() );
}
