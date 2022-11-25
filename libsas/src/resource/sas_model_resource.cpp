#include "sas_pch.h"
#include "sas_model_resource.h"
#include "render/sas_mesh.h"
#include "render/sas_geometry.h"
#include "render/sas_instance.h"
#include "render/sas_vertexbuffer.h"
#include "render/sas_indexbuffer.h"
#include "render/sas_material.h"
#include "utils/sas_camera.h"
#include "animation/sas_skeleton.h"
#include "sas_resourcemng.h"

sasModelResource::sasModelResource( const sasModelResourceData& data )
    : _data( data )
{
}

sasModelResource::~sasModelResource()
{
  //remove itself from resource mng model list
  sasResourceMng* rscMng = sasResourceMng::singletonPtr();
  rscMng->unregisterModelRsc( this );

  //delete internals
  for( size_t i=0; i<_data.vertexBuffers.size(); i++ )
    sasDelete _data.vertexBuffers[i];
  for( size_t i=0; i<_data.indexBuffers.size(); i++ )
    sasDelete _data.indexBuffers[i];
  for( size_t i=0; i<_data.subMeshes.size(); i++ )
    sasDelete _data.subMeshes[i];
  for( size_t i=0; i<_data.skeletonMappings.size(); i++ )
    sasDelete _data.skeletonMappings[i];
  for( size_t i=0; i<_data.skeletons.size(); i++ )
    sasDelete _data.skeletons[i];
  for( size_t i=0; i<_data.meshes.size(); i++ )
    sasDelete _data.meshes[i];
  for( size_t i=0; i<_data.geometries.size(); i++ )
    sasDelete _data.geometries[i];
  for( size_t i=0; i<_data.instances.size(); i++ )
    sasDelete _data.instances[i];  
  for( size_t i=0; i<_data.cameras.size(); i++ )
    sasDelete _data.cameras[i];  
  for( size_t i=0; i<_data.materials.size(); i++ )
    sasDelete _data.materials[i];  

  sasDelete _data.skeletonHierarchy;
}
