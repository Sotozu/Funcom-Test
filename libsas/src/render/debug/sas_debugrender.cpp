#include "sas_pch.h"
#include "sas_debugrender.h"
#include "sas_debugmng.h"
#include "render/sas_rendermng.h"
#include "sas_device.h"
#include "render/sas_rendertarget.h"
#include "render/sas_depthbuffer.h"
#include "render/shaders/sas_shadermng.h"
#include "render/sas_settings.h"
#include "sas_device.h"
#include "render/sas_constantbuffer.h"
#include "utils/sas_camera.h"
#include "render/sas_miscresources.h"
#include "render/shaders/sas_shaderids.h"
#include "render/sas_vertexbuffer.h"
#include "utils/sas_frustum.h"
#include "utils/sas_color.h"
#include "resource/sas_resourcemng.h"
#include "resource/sas_modelinstance.h"
#include "animation/sas_skeleton.h"
#include "render/sas_instance.h"
#include "render/sas_indexbuffer.h"

struct LineConstants
{
  smVec4 _posA;
  smVec4 _posB;
  smVec4 _col;
};

struct VolumeTexConstants
{
  smVec4 _slice; //yzw unused
};

sasDebugRender::sasDebugRender()
  : _numDebugSpheres( 0 )
  , _numDebugLines( 0 )
  , _numDebugSkeletons( 0 )
  , _numDebugBones( 0 )
  , _numDebugBoxes( 0 )
{
  _lineCB = sasNew sasConstantBuffer( sizeof( LineConstants ) );
  _volumeTexCB = sasNew sasConstantBuffer( sizeof( VolumeTexConstants ) );
  _boxCB = sasNew sasConstantBuffer( sizeof( sasPerInstanceConstants ) );
}

sasDebugRender::~sasDebugRender()
{
  sasDelete _boxCB;
  sasDelete _volumeTexCB;
  sasDelete _lineCB;
}

void sasDebugRender::renderFrustum( const sasFrustumPoints& frustum )
{
  sasDebugMng::singletonPtr()->renderFrustum( sasDevice::singleton(), frustum );
}

void sasDebugRender::renderBox( const smAABB& aabb, const smMat44& xform, const smVec4& colour )
{
  sasDevice* device = sasDevice::singletonPtr();
  sasMiscResources* mrd = sasMiscResources::singletonPtr();

  float scaleX = aabb._max.X - aabb._min.X;
  float scaleY = aabb._max.Y - aabb._min.Y;
  float scaleZ = aabb._max.Z - aabb._min.Z;
  smMat44 scaleXform;
  smGetScalingMatrix( scaleX, scaleY, scaleZ, scaleXform );

  smVec3 centre = aabb.centrePoint();
  smMat44 offsetMtx;
  offsetMtx.setIdentity();
  smSetTranslation( centre, offsetMtx );
  smMul( offsetMtx, scaleXform, scaleXform );

  smMat44 worldXform;
  smMul( xform, scaleXform, worldXform );
  sasPerInstanceConstants* cbData = static_cast< sasPerInstanceConstants* >( _boxCB->map() );
  if( cbData )
  {
    cbData->_worldMtx = worldXform;
    cbData->_lastFrameWorldMtx = worldXform;
    cbData->_instanceId = smVec4( 0.f );
    cbData->_tintColour = colour;
  }
  _boxCB->unmap();

  device->setIndexBuffer( mrd->getUnitCubeIB() );        
  device->setVertexBuffer( mrd->getUnitCubeVB() );      
  device->setConstantBuffer( 1, _boxCB, sasDeviceUnit::VertexShader );  
  device->setConstantBuffer( 1, _boxCB, sasDeviceUnit::PixelShader );   

  device->setVertexShader( defaultShaderID | debugDiffuseOnlyMask );
  device->setPixelShader( defaultShaderID | debugDiffuseOnlyMask );
  device->drawIndexed( sasPrimTopology_TriList, mrd->getUnitCubeIB()->indicesCount() / 3 );
}

void sasDebugRender::renderLine( const smVec3& pos_a, const smVec3& pos_b, const sasColor& col, sasDepthStencilState depthState )
{
  sasDevice* device = sasDevice::singletonPtr();

  LineConstants* lineConsts = static_cast< LineConstants* >( _lineCB->map() );
  lineConsts->_col.X = col.R;
  lineConsts->_col.Y = col.G;
  lineConsts->_col.Z = col.B;
  lineConsts->_col.W = col.A;
  lineConsts->_posA = smVec4( pos_a );
  lineConsts->_posB = smVec4( pos_b );
  _lineCB->unmap();

  device->setDepthStencilState( depthState, 0 );
  device->setDepthStencilTarget( device->depthBuffer(), false );
  device->setVertexBuffer( NULL );
  device->setIndexBuffer( NULL );
  device->setConstantBuffer( 1, _lineCB, sasDeviceUnit::VertexShader );
  device->setVertexShader( renderLineShaderID );
  device->setPixelShader( renderLineShaderID );
  device->draw( sasPrimTopology_LineList, 2 );
}

void sasDebugRender::renderSphere( const smVec3& pos, float radius, const smVec4& colour )
{
  smMat44 t;
  smEulerAngles r;
  smGetXFormTRSMatrix( pos.X, pos.Y, pos.Z, r, radius, t );
  sasRenderMng::singletonPtr()->renderBoundingSphere( t, colour );
}

void sasDebugRender::renderVolumeTextureSlice( sasRenderTarget* volumeTexture, unsigned int slice )
{
  sasASSERT( volumeTexture );
  sasASSERT( volumeTexture->depth() );

  INSERT_PIX_EVENT("PIXEvent: render 3d tex slice", 0xffffffff);
  sasDevice* device = sasDevice::singletonPtr();

  VolumeTexConstants* consts = static_cast< VolumeTexConstants* >( _volumeTexCB->map() );
  consts->_slice = smVec4( static_cast< float >( slice ), static_cast< float >( volumeTexture->width() ), 
                            static_cast< float >( volumeTexture->height() ), static_cast< float >( volumeTexture->depth() ) );
  _volumeTexCB->unmap();

  device->setConstantBuffer( 1, _volumeTexCB, sasDeviceUnit::PixelShader );
  device->setVertexShader( render3DTexSliceShaderID );
  device->setPixelShader( render3DTexSliceShaderID );
  device->setVertexBuffer( sasMiscResources::singletonPtr()->getFullScreenQuadVB() );
  device->setTexture( 0, volumeTexture, sasDeviceUnit::PixelShader );
  device->setSamplerState( 0, sasSamplerState_Bilinear_Clamp, sasDeviceUnit::PixelShader );
  device->setDepthStencilTarget( NULL, false );
  device->draw( sasPrimTopology_TriStrip, 4 );
  device->setTexture( 0, static_cast< sasTexture* >( NULL ), sasDeviceUnit::PixelShader );
  device->flushStates();
}

void sasDebugRender::queueRenderSphere( const sasDebugSphere& s )
{
  sasASSERT( _numDebugSpheres < ( kMaxDebugSpheres - 1 ) );
  _debugSphereQueue[ _numDebugSpheres ] = s;
  _numDebugSpheres++;
}

void sasDebugRender::queueRenderBox( const sasDebugBox& b )
{
  sasASSERT( _numDebugBoxes < ( kMaxDebugBoxes - 1 ) );
  _debugBoxQueue[ _numDebugBoxes ] = b;
  _numDebugBoxes++;
}

void sasDebugRender::queueRenderLine( const sasDebugLine& l )
{
  sasASSERT( _numDebugLines < ( kMaxDebugLines - 1 ) );
  _debugLineQueue[ _numDebugLines ] = l;
  _numDebugLines++;
}

void sasDebugRender::queueRenderSkeleton( const sasStringId& modelInstId )
{
  sasModelInstance* modelInst = sasResourceMng::singletonPtr()->findModelInstance( modelInstId );
  if( modelInst && modelInst->skinned() )
  {
    sasASSERT( _numDebugSkeletons < ( kMaxDebugSkeletons - 1 ) );
    _debugSkeletonQueue[ _numDebugSkeletons ] = modelInst;
    _numDebugSkeletons++;
  }
}

void sasDebugRender::queueRenderBone( const sasStringId& modelInstId, const sasStringId& boneId )
{
  sasModelInstance* modelInst = sasResourceMng::singletonPtr()->findModelInstance( modelInstId );
  if( modelInst && modelInst->skinned() )
  {
    for( size_t i = 0; i < modelInst->numInstances(); i++ )
    {
      sasInstance* inst = modelInst->instance( i );
      if( inst )
      {
        for( uint32_t sm = 0; sm < inst->numSubmeshes(); sm++ )
        {
          const sasSkeleton* sk = inst->skeleton( sm );
          if( sk )
          {
            uint32_t dummyIndex;
            if( sk->findBoneIndexSafe( boneId, dummyIndex ) )
            {
              smVec3 bonePos = sk->getBoneWorldPosition( boneId );
              smMul( inst->worldMatrix(), bonePos, bonePos );

              sasASSERT( _numDebugBones < ( kMaxDebugBones - 1 ) );
              _debugBoneQueue[ _numDebugBones ] = bonePos;
              _numDebugBones++;
              return;
            }
          } 
        }
      }
    }
  }
}

void sasDebugRender::flushQueuedDebugObjects( sasRenderTarget* rt, sasDepthStencilTarget* dst )
{
  sasDevice* d = sasDevice::singletonPtr();
  d->setDepthStencilTarget( dst, false );
  d->setRenderTarget( 0, rt );
  d->setViewport( rt );

  for( uint32_t i = 0; i < _numDebugSpheres; i++ )
  {
    d->setDepthStencilState( _debugSphereQueue[ i ].depthState, 0 );
    d->setRasterizerState( _debugSphereQueue[ i ].rasterState );
    renderSphere( _debugSphereQueue[ i ]._pos, _debugSphereQueue[ i ]._radius, _debugSphereQueue[ i ]._col );
  }

  for( uint32_t i = 0; i < _numDebugBoxes; i++ )
  {
    d->setDepthStencilState( _debugBoxQueue[ i ].depthState, 0 );
    d->setRasterizerState( _debugBoxQueue[ i ].rasterState );
    renderBox( _debugBoxQueue[ i ]._aabb, _debugBoxQueue[ i ]._xform, _debugBoxQueue[ i ]._col );
  }

  d->setRasterizerState( sasRasterizerState_Solid );

  for( uint32_t i = 0; i < _numDebugLines; i++ )
  {
    sasColor col( _debugLineQueue[ i ]._col.X, _debugLineQueue[ i ]._col.Y, _debugLineQueue[ i ]._col.Z, _debugLineQueue[ i ]._col.W );
    renderLine( _debugLineQueue[ i ]._posA, _debugLineQueue[ i ]._posB, col, _debugLineQueue[ i ].depthState );
  }

  for( uint32_t i = 0; i < _numDebugSkeletons; i++ )
  {
    if( _debugSkeletonQueue[ i ] )
      _debugSkeletonQueue[ i ]->renderSkeleton();
  }

  d->setBlendState( sasBlendState_One_One );
  d->setRasterizerState( sasRasterizerState_Wireframe );
  for( uint32_t i = 0; i < _numDebugBones; i++ )
  {
    d->setDepthStencilState( sasDepthStencilState_NoZTest, 0 );
    renderSphere( _debugBoneQueue[ i ], 0.08f, smVec4( 0.f, 1.f, 0.334f, 0.334f ) );  
  }
  d->setRasterizerState( sasRasterizerState_Solid );
  d->setBlendState( sasBlendState_Opaque );

  _numDebugSkeletons = 0;
  _numDebugLines = 0;
  _numDebugSpheres = 0;
  _numDebugBones = 0;
  _numDebugBoxes = 0;
  d->setDepthStencilState( sasDepthStencilState_LessEqual, 0 );
}
