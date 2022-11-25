#include "sas_pch.h"
#include "sas_editor.h"
#include "render/sas_constantbuffer.h"
#include "render/sas_rendertarget.h"
#include "render/sas_rendertargetpool.h"
#include "render/sas_rendermng.h"
#include "sas_device.h"
#include "utils/sas_timer.h"
#include "utils/sas_color.h"
#include "render/sas_structuredbuffer.h"
#include "render/shaders/sas_shaderids.h"
#include "render/sas_instance.h"
#include "resource/sas_splashscreen.h"
#include "resource/sas_resourcemng.h"
#include "utils/sas_cameramng.h"
#include "utils/sas_camera.h"
#include "render/debug/sas_debugrender.h"
#include "render/lights/sas_light.h"
#include "render/lights/sas_lightmng.h"
#include "resource/sas_camerapath.h"
#include "render/debug/sas_debugmng.h"
#include "utils/sas_culling.h"
#include "resource/sas_modelinstance.h"
#include "animation/sas_skeleton.h"
#include "render/sas_settings.h"

#ifdef PLATFORM_DX11
#include "../dx11/sas_device_buffer.h"
#endif

struct sasPickingConstants
{
  smVec4 mousePos; //zw unused
};

sasEditor::sasEditor()
  : _enabled( false )
  , _lastPickingTimestamp( 0.f )
  , _pickMeshThisFrame( false )
  , _readbackRequired( false )
  , _meshIdBuffer( NULL )
  , _pickedBoneDataState( sasEditorQueryState::Inactive )
  , _queryX( 0 )
  , _queryY( 0 )
  , _queryMeshId( 0 )
{
  _constantBuffer = sasNew sasConstantBuffer( sizeof( sasPickingConstants ) );
  _readbackBuffer = sasNew sasStructuredBuffer( sizeof( float ) * 4, sizeof( float ) * 4, false, false );
  _stagingBuffer = sasNew sasStructuredBuffer( sizeof( float ) * 4, sizeof( float ) * 4, false, false, true );
}

sasEditor::~sasEditor()
{
  sasDelete _stagingBuffer;
  sasDelete _readbackBuffer;
  sasDelete _constantBuffer;
}

void sasEditor::frameInitialize()
{
  if( !_enabled )
    return;

  sasRenderTargetDesc rtDesc;
  rtDesc.format = sasPixelFormat::R_F32;
  rtDesc.mips = 1;
  rtDesc.needUAV = false;
  rtDesc.type = sasTextureType::sasTexture2D;
  rtDesc.width = sasSettings::singletonPtr()->_frameBufferResolution.width(); 
  rtDesc.height = sasSettings::singletonPtr()->_frameBufferResolution.height();

  _meshIdBuffer = sasRenderTargetPool::singletonPtr()->RequestAndLock( rtDesc );

  sasDevice::singletonPtr()->setRenderTarget( 3, _meshIdBuffer );

  sasColor clearColor;
  clearColor.set( 0.f, 0.f, 0.f );
  sasDevice::singletonPtr()->clearRenderTarget( _meshIdBuffer, &clearColor );
}

void sasEditor::frameEnd()
{
  if( _meshIdBuffer == NULL )
    return;

  sasRenderTargetPool::singletonPtr()->Unlock( _meshIdBuffer );
  _meshIdBuffer = NULL;
}

void sasEditor::step()
{
  if( !enabled() )
    return;

  sasDevice* device = sasDevice::singletonPtr();

  bool updateMeshBarData = false;
  if( _readbackRequired )
  {
    _readbackRequired = false;
    updateMeshBarData = true;
    device->copyResource( _stagingBuffer, _readbackBuffer );
    
#ifdef PLATFORM_DX11
    sasDeviceBuffer* db = static_cast< sasDeviceBuffer* >( _stagingBuffer->deviceObject() );
    D3D11_MAPPED_SUBRESOURCE msr;
    device->d3dDeviceContext()->Map( db->d3dBuffer(), 0, D3D11_MAP_READ, 0, &msr );
    float* data = static_cast< float* >( msr.pData );

    //sasDebugOutput( "Picked instance ID: %f\n", data[ 0 ] );
    _pickedMeshInstanceId = static_cast< unsigned int >( data[ 0 ] );

    device->d3dDeviceContext()->Unmap( db->d3dBuffer(), 0 );
#endif
  }

  if( _pickedMeshDataState == sasEditorQueryState::Issued )
  {
    _pickedMeshDataState = sasEditorQueryState::Resolved;

    sasInstance* inst = NULL;
    sasInstance::Iterator instanceIt  =   sasInstance::begin();
    sasInstance::Iterator last        =   sasInstance::end();
    while( instanceIt != last )
    {
      if( ( *instanceIt )->uid() == _pickedMeshInstanceId )
      {
        inst = *instanceIt;
        break;
      }

      instanceIt++;
    }

    if( inst != nullptr )
    {
      sasModelInstance* modelInst = inst->parentModelInstance();
      if( inst && modelInst )
      {
        _pickedMeshData._meshId = _pickedMeshInstanceId;
        _pickedMeshData._modelInstanceId = modelInst->instanceId();
        _pickedMeshData._modelPos = modelInst->position();
        _pickedMeshData._modelRot = modelInst->rotation();
        _pickedMeshData._modelScale = modelInst->scale();
        _pickedMeshData._skinned = modelInst->skinned();
        _pickedMeshData._meshBoundingSphere = *( inst->wsBoundingSphere() );
      }
    }
    else
    {
      _pickedMeshDataState = sasEditorQueryState::Inactive;
    }
    
  }

}

void sasEditor::runComputeJobs()
{
  if( !_enabled || !_pickMeshThisFrame )
    return;

  sasDevice* device = sasDevice::singletonPtr();
  device->setComputeShader( meshPickingShaderID );
  device->setTexture( 0, _meshIdBuffer, sasDeviceUnit::ComputeShader );
  device->setConstantBuffer( 1, _constantBuffer, sasDeviceUnit::ComputeShader );
  device->setUAV( 0, _readbackBuffer, sasDeviceUnit::ComputeShader );

  device->dispatch( 1, 1, 1 );

  sasShaderID nullShader;
  device->setComputeShader( nullShader );
  device->setConstantBuffer( 1, NULL, sasDeviceUnit::ComputeShader );
  device->setTexture( 0, static_cast<sasRenderTarget*>(NULL), sasDeviceUnit::ComputeShader );
  device->setUAV( 0, static_cast<sasRenderTarget*>(NULL), sasDeviceUnit::ComputeShader );

  //sasDebugOutput( "Picked mesh! w00t!\n");

  _pickMeshThisFrame = false;
  _readbackRequired = true;
}

void sasEditor::setEnabled( bool state )
{ 
  _enabled = state; 
  if( !state ) 
  {
    _pickedMeshInstanceId = 0;
  }
}

void sasEditor::kickMeshPickingTask( uint32_t x, uint32_t y )
{
  _pickedMeshDataState = sasEditorQueryState::Issued;
  _queryX = x;
  _queryY = y;

  _pickMeshThisFrame = true;

  sasPickingConstants* cb = static_cast< sasPickingConstants* >( _constantBuffer->map() );
  cb->mousePos = smVec4(  static_cast<float>( x ) / static_cast< float >( sasDevice::singletonPtr()->backBufferWidth() ), 
                          static_cast<float>( y ) / static_cast< float >( sasDevice::singletonPtr()->backBufferHeight() ),
                          0.f, 0.f );
  _constantBuffer->unmap(); 
}

void sasEditor::kickBonePickTaskForInstance( uint32_t x, uint32_t y, sasStringId modelId, uint32_t meshId )
{
  _pickedBoneDataState = sasEditorQueryState::Inactive; //resolved immediately, false by default
  _queryX = x;
  _queryY = y;
  _queryMeshId = meshId;
  _queryModelId = modelId;

  //pick bones if debugging skeleton
  //run cpu intersection tests with bone spheres
  if( meshId != 0 )
  {
    sasInstance* inst = NULL;
    sasInstance::Iterator instanceIt  =   sasInstance::begin();
    sasInstance::Iterator last        =   sasInstance::end();
    while( instanceIt != last )
    {
      if( ( *instanceIt )->uid() == _pickedMeshInstanceId )
      {
        inst = *instanceIt;
        break;
      }

      instanceIt++;
    }

    if( inst )
    {
      //generate ray
      smVec4 screenSpacePos = smVec4(  ( static_cast<float>( x ) / static_cast< float >( sasDevice::singletonPtr()->backBufferWidth() ) ) * 2.f - 1.f, 
                                        1.f - ( static_cast<float>( y ) / static_cast< float >( sasDevice::singletonPtr()->backBufferHeight() ) ) * 2.f,
                                        1.f, 1.f );

      smMat44 matView;
      smMat44 matProj;
      sasCamera* camera = sasCameraMng::singleton().getCurrentCamera();
      sasASSERT( camera );

      camera->getViewMatrix( matView );
      camera->getProjMatrix( (float)sasDevice::singletonPtr()->backBufferWidth(), (float)sasDevice::singletonPtr()->backBufferHeight(), matProj );
      smMat44 matProjView;
      smMul( matProj, matView, matProjView );
      smMat44 matInvProjView;
      smInverse( matProjView, matInvProjView );

      smVec4 rayDir;
      smVec4 worldPos; 
      smMul( matInvProjView, screenSpacePos, worldPos );
      worldPos /= worldPos.W;
      smNormalize3( worldPos - camera->position(), rayDir );

      for( unsigned int i = 0; i < inst->numSubmeshes(); i++ )
      {
        const sasSkeleton* sk = inst->skeleton( i );
        if( sk )
        {
          for( unsigned int b = 0; b < sk->numBones(); b++ )
          {
            sasStringId boneId = sk->boneId( b );
            smVec3 bonePosLS = sk->getBoneWorldPosition( boneId );
            smVec3 bonePosWS;
            smMul( inst->worldMatrix(), bonePosLS, bonePosWS );
            float dist = 0.f;
            if( sasIntersetRaySphere( camera->position(), rayDir, bonePosWS, 0.05f, dist ) )
            {
              _pickedBoneData._boneId = boneId;
              _pickedBoneData._meshId = meshId;
              _pickedBoneData._modelInstanceId = modelId;
              _pickedBoneData._submeshIndex = i;
              _pickedBoneDataState = sasEditorQueryState::Resolved;
              return;
            }
          }
        }
      }
    }
  }
}

const sasPickedMeshData& sasEditor::pickedMeshData() const
{
  return _pickedMeshData;
}

bool sasEditor::areMeshPickingResultsReady() const
{
  return ( _pickedMeshDataState == sasEditorQueryState::Resolved );
}

const sasPickedBoneData& sasEditor::pickedBoneData() const
{
  return _pickedBoneData;
}

bool sasEditor::areBonePickingResultsReady() const
{
  return ( _pickedBoneDataState == sasEditorQueryState::Resolved );
}

