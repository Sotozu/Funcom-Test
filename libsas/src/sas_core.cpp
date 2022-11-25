#include "sas_pch.h"
#include "sas_core.h"
#include "render/sas_rendermng.h"
#include "resource/sas_resourcemng.h"
#include "render/lights/sas_lightmng.h"
#include "utils/sas_timer.h"
#include "utils/sas_cameramng.h"
#include "render/sas_miscresources.h"
#include "utils/sas_hash.h"
#include "sas_cmp.h"

extern void sasAssimpRegisterLoader();
extern void sasTextureRegisterLoader();
extern void sasModelRegisterLoader();

sasCore::sasCore( const sasConfig& config )
  : _renderMng(0)
  , _resourceMng(0)
  , _cameraMng(0)
{  
  _contiguousMemPool = new sasContiguousMemPool();
  _resourceMng  = sasNew sasResourceMng( config.resourcePath );   
  _renderMng    = sasNew sasRenderMng( config ); 
  _cameraMng    = sasNew sasCameraMng();
  _lightMng     = sasNew sasLightMng();

  sasAssimpRegisterLoader();
  sasModelRegisterLoader();
  sasTextureRegisterLoader();

  //load engine required textures
  _renderMng->loadEngineResources();
}

sasCore::~sasCore()
{
  // reverse order
  sasDelete _lightMng;
  sasDelete _cameraMng;
  sasDelete _renderMng;    
  sasDelete _resourceMng;
  sasDelete _contiguousMemPool;
}

void sasCore::update()
{
  _cameraMng->step();
  _renderMng->update();
}