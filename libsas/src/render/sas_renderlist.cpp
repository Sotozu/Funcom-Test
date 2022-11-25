#include "sas_pch.h"
#include "sas_renderlist.h"
#include "sas_rendermng.h"
#include "Visibility/sas_drawlist.h"
#include "sas_geometry.h"
#include "sas_instance.h"
#include "sas_mesh.h"
#include "sas_device.h"
#include "sas_material.h"
#include "sas_constantbuffer.h"
#include "sas_vertexformat.h"
#include "sas_vertexbuffer.h"
#include "shaders/sas_shaderids.h"
#include "resource/sas_texture_resource.h"
#include "render/sas_settings.h"
#include "render/debug/sas_debugmng.h"
#include "render/sas_miscresources.h"
#include "editor/sas_editor.h"
#include "utils/sas_sort.h"
#include "utils/sas_hash.h"
 
 void sasRenderList::renderDrawListImmediate( sasDrawList* drawlist, unsigned int shaderFlagMask, smMat44* matProjView, sasConstantBuffer* constantBuffer, sasRenderMode renderMode )
 {
   sasShaderID nullShader;
   sasDevice* device = sasDevice::singletonPtr();
 
   if( renderMode == sasRenderMode_Wireframe )
   {
     device->setRasterizerState( sasRasterizerState_Wireframe );
     renderMode = sasRenderMode_default;
   }
   
   sasShaderID uberShaderID = defaultShaderID;
   bool bUseGS = false;
   bool bTessellation = sasSettings::singletonPtr()->_tessellationEnabled;
   if( shaderFlagMask == sasRenderList_ShadowPass ) //shadow pass
   {
     bTessellation = false;
     uberShaderID = uberShaderID /*| debugDiffuseOnlyMask*/;
     device->setBlendState( sasBlendState_Opaque_NoColourWrites );
   }
   else if( shaderFlagMask == sasRenderList_CubemapPass ) //cubemap generation pass
   {
     bTessellation = false;
     uberShaderID = uberShaderID | debugDiffuseOnlyMask | cubemapGSOutputMask;
     bUseGS = true;
   }
   else //gbuffer pass
   {
     uberShaderID = uberShaderID;// | cubeReflectionMask;
     if( sasEditor::singletonPtr()->enabled() )
     {
       uberShaderID = uberShaderID | meshIdMask;
     }
   }
 
   sasPrimTopology topology = sasPrimTopology_TriList;
   if( bTessellation )
   {
     uberShaderID = uberShaderID | tessellationMask;
     if( sasSettings::singletonPtr()->_tessellationMode == sasTessellationSmoothing::Phong )
     {
       uberShaderID = uberShaderID | tessellationPhongSmoothingMask;
     }
     else if( sasSettings::singletonPtr()->_tessellationMode == sasTessellationSmoothing::PNTriangles )
     {
       uberShaderID = uberShaderID | tessellationPNTSmoothingMask;
     }
   }
 
   if( sasSettings::singletonPtr()->_motionBlurEnabled && ( shaderFlagMask != sasRenderList_CubemapPass ) )
   {
     uberShaderID = uberShaderID | velocityOutputMask;
   }
 
   bool showUVs = sasDebugMng::singletonPtr()->showUVs();
   if( showUVs )
   {
     uberShaderID = uberShaderID | debugUVsMask;
   }
 
   for( unsigned int i = 0; i < drawlist->_size; i++ )
   {
     sasInstance* instance = drawlist->_drawList[i];
     sasGeometry* geom = instance->geometry();
     sasMesh* mesh = geom->lod( 0 );
 
     void* ptr = constantBuffer->map();
     if( ptr )
     {
       sasPerInstanceConstants* cbData = static_cast< sasPerInstanceConstants* >( ptr );
       cbData->_worldMtx = instance->worldMatrix();
       cbData->_lastFrameWorldMtx = instance->lastFrameWorldMatrix();
       cbData->_instanceId = smVec4( instance->uidFloat(), 0.f, 0.f, 0.f );
       cbData->_tintColour = instance->tintColour();
       constantBuffer->unmap();
     }
 
     for( size_t n=0; n<mesh->subMeshCount(); n++ )
     {
       sasSubMesh* subMesh = mesh->subMesh( n );
 
       if((renderMode == sasRenderMode_default))
       {
 
         device->setIndexBuffer( subMesh->indexBuffer() );        
         device->setVertexBuffer( subMesh->vertexBuffer() );      
         device->setConstantBuffer( 1, constantBuffer, sasDeviceUnit::VertexShader );   
         device->setConstantBuffer( 1, constantBuffer, sasDeviceUnit::PixelShader );  //instance id is used in PS
 
         sasMaterial* material = subMesh->material();
         size_t nLayers = 0;
 
         sasShaderID materialShaderID = uberShaderID;
 
         if( material )
         {
           nLayers = material->layerCount();
           size_t layerIndex = 0;
           while(  layerIndex<nLayers )
           {
             sasMaterialLayer* layer = material->layer( layerIndex );
             device->setTexture( material->layer( layerIndex )->getLayerTextureIndex(), layer->texture ? layer->texture->texture() : 0, sasDeviceUnit::PixelShader );
             device->setSamplerState( layerIndex, sasSamplerState_Anisotropic_Wrap, sasDeviceUnit::PixelShader );
             ++layerIndex;
           }
 
           /*if( sasEditor::singletonPtr()->enabled() && instance->uid() == sasEditor::singletonPtr()->pickedInstanceId() )
           {
             device->setTexture( 0, sasMiscResources::singletonPtr()->getSelectedTexture(), sasDeviceUnit::PixelShader );
           }*/
 
           material->applyMaterialShaderFlags( materialShaderID );
         }
 
         
         if( subMesh->vertexBuffer()->format().tangent == sasVertexFormat::TANGENT_3F )
         {
           materialShaderID = materialShaderID | hasTangentMask;
         }

         if( subMesh->vertexBuffer()->format().tangent == sasVertexFormat::UV_2F )
         {
           materialShaderID = materialShaderID | hasUVsMask;
         }
 
         if( subMesh->vertexBuffer()->format().boneIndices == sasVertexFormat::BONEINDICES )
         {
           materialShaderID = materialShaderID | hasBoneDataMask;
           device->setStructuredBuffer( 2, instance->skinningMatricesBuffer( n ), sasDeviceUnit::VertexShader );
         }
 
         if(bUseGS)
         {
           materialShaderID = materialShaderID | geomShaderMask;
           device->setGeometryShader( materialShaderID );
         }
 
         device->setVertexShader( materialShaderID );
         if( shaderFlagMask == sasRenderList_ShadowPass )
         {
           device->setPixelShader( nullShader );
         }
         else
         {
           device->setPixelShader( materialShaderID );
         }
         
         if(bTessellation)
         {
           device->setHullShader( materialShaderID );
           device->setDomainShader( materialShaderID );
           topology = sasPrimTopology_TriPatch;
         }
 
         device->drawIndexed( topology, subMesh->primitiveCount() );
       }
       else if ( renderMode == sasRenderMode_BoundingSpheres )
       {
         smMat44 t;
         smEulerAngles r;
         const smVec4* bSphere = instance->wsBoundingSphere();
         smGetXFormTRSMatrix( bSphere->X, bSphere->Y, bSphere->Z, r, bSphere->W, t );
         smVec4 colour( 0.8f, 0.2f, 0.35f, 0.5f );
         sasRenderMng::singletonPtr()->renderBoundingSphere( t, colour );
       }
     }
   }
 
   device->setHullShader( nullShader );
   device->setDomainShader( nullShader );
   device->setGeometryShader( nullShader );
   device->setRasterizerState( sasRasterizerState_Solid );
   device->setBlendState( sasBlendState_Null );
 }

void sasRenderList::processList( sasDrawList* drawlist, unsigned int shaderFlagMask, smMat44* matProjView, const smVec3& cameraPos, sasRenderMode renderMode )
{
  sasShaderID nullShader;
  clearList();

  sasRasterizerState rs     = sasRasterizerState_Solid;
  sasBlendState bs          = sasBlendState_Null;
  sasDepthStencilState dss  = sasDepthStencilState_Null;

  if( renderMode == sasRenderMode_Wireframe )
  {
    rs = sasRasterizerState_Wireframe;
    renderMode = sasRenderMode_default;
  }

  sasShaderID uberShaderID = defaultShaderID;
  bool bUseGS = false;
  bool bTessellation = sasSettings::singletonPtr()->_tessellationEnabled;
  if( shaderFlagMask == sasRenderList_ShadowPass ) //shadow pass
  {
    bTessellation = false;
    uberShaderID = uberShaderID /*| debugDiffuseOnlyMask*/;
    bs = sasBlendState_Opaque_NoColourWrites;
  }
  else if( shaderFlagMask == sasRenderList_CubemapPass ) //cubemap generation pass
  {
    bTessellation = false;
    uberShaderID = uberShaderID | debugDiffuseOnlyMask | cubemapGSOutputMask;
    bUseGS = true;
  }
  else //gbuffer pass
  {
    uberShaderID = uberShaderID;// | cubeReflectionMask;
    if( sasEditor::singletonPtr()->enabled() )
    {
      uberShaderID = uberShaderID | meshIdMask;
    }
  }

  sasPrimTopology topology = sasPrimTopology_TriList;
  if( bTessellation )
  {
    uberShaderID = uberShaderID | tessellationMask;
    if( sasSettings::singletonPtr()->_tessellationMode == sasTessellationSmoothing::Phong )
    {
      uberShaderID = uberShaderID | tessellationPhongSmoothingMask;
    }
    else if( sasSettings::singletonPtr()->_tessellationMode == sasTessellationSmoothing::PNTriangles )
    {
      uberShaderID = uberShaderID | tessellationPNTSmoothingMask;
    }
  }

  if( sasSettings::singletonPtr()->_motionBlurEnabled && ( shaderFlagMask != sasRenderList_CubemapPass ) )
  {
    uberShaderID = uberShaderID | velocityOutputMask;
  }

  bool showUVs = sasDebugMng::singletonPtr()->showUVs();
  if( showUVs )
  {
    uberShaderID = uberShaderID | debugUVsMask;
  }

  for( unsigned int i = 0; i < drawlist->_size; i++ )
  {
    sasInstance* instance = drawlist->_drawList[i];
    sasGeometry* geom = instance->geometry();
    sasMesh* mesh = geom->lod( 0 );

    //per instance constants 
    sasPerInstanceConstants constants;
    constants._worldMtx = instance->worldMatrix();
    constants._lastFrameWorldMtx = instance->lastFrameWorldMatrix();
    constants._instanceId = smVec4( instance->uidFloat(), 0.f, 0.f, 0.f );
    constants._tintColour = instance->tintColour();

    //per mesh data
    for( size_t n = 0; n < mesh->subMeshCount(); n++ )
    {
      sasSubMesh* subMesh = mesh->subMesh( n );
      
      if( renderMode == sasRenderMode_default )
      {
        sasModelRenderBlob& mrb = _renderBlobs[ _numRenderBlobs++ ];
        mrb._stageMask = sasStageVertexShader;
        mrb._ib = subMesh->indexBuffer();        
        mrb._vb = subMesh->vertexBuffer();      
        mrb._constants = constants;

        sasMaterialType::Enum matType = sasMaterialType::Opaque;
        sasMaterial* material = subMesh->material();
        size_t nLayers = 0;

        sasShaderID materialShaderID = uberShaderID;

        if( material )
        {
          matType = material->materialType();
          nLayers = material->layerCount();
          size_t layerIndex = 0;
          while(  layerIndex<nLayers )
          {
            sasMaterialLayer* layer = material->layer( layerIndex );
            
            sasTextureBlob texBlob;
            texBlob._index = material->layer( layerIndex )->getLayerTextureIndex();
            texBlob._texture = layer->texture ? layer->texture->texture() : 0;
            texBlob._stage = sasDeviceUnit::PixelShader;
            mrb.pushBlob( texBlob );
            
            sasSamplerStateBlob samplerBlob;
            samplerBlob._index = layerIndex;
            samplerBlob._samplerState = sasSamplerState_Anisotropic_Wrap;
            samplerBlob._stage = sasDeviceUnit::PixelShader;
            mrb.pushBlob( samplerBlob );

            ++layerIndex;
          }

          /*if( sasEditor::singletonPtr()->enabled() && instance->uid() == sasEditor::singletonPtr()->pickedInstanceId() )
          {
            //set special debug texture for currently selected instance. todo: disable in final build
            sasTextureBlob texBlob;
            texBlob._index = 0;
            texBlob._texture = sasMiscResources::singletonPtr()->getSelectedTexture();
            texBlob._stage = sasDeviceUnit::PixelShader;
            mrb.pushBlob( texBlob );
          }*/

          material->applyMaterialShaderFlags( materialShaderID );
        }


        if( subMesh->vertexBuffer()->format().tangent == sasVertexFormat::TANGENT_3F )
        {
          materialShaderID = materialShaderID | hasTangentMask;
        }

        if( subMesh->vertexBuffer()->format().tangent == sasVertexFormat::UV_2F )
        {
          materialShaderID = materialShaderID | hasUVsMask;
        }

        if( subMesh->vertexBuffer()->format().boneIndices == sasVertexFormat::BONEINDICES )
        {
          materialShaderID = materialShaderID | hasBoneDataMask;

          //set skinning matrices
          sasStructuredBufferBlob structBufferBlob;
          structBufferBlob._index = 2; //Skinning matrices slot
          structBufferBlob._sb = instance->skinningMatricesBuffer( n );
          structBufferBlob._stage = sasDeviceUnit::VertexShader;
          mrb.pushBlob( structBufferBlob );
        }

        if( instance->instancedData() != nullptr )
        {
            materialShaderID = materialShaderID | hasInstancedXfms;

            sasStructuredBufferBlob structBufferBlob;
            structBufferBlob._index = 3; //Skinning matrices slot
            structBufferBlob._sb = instance->instancedData();
            structBufferBlob._stage = sasDeviceUnit::VertexShader;
            mrb.pushBlob( structBufferBlob );
            mrb._instanceCount = instance->instanceCount();
        }
        else
            mrb._instanceCount = 1;

        if(bUseGS)
        {
          materialShaderID = materialShaderID | geomShaderMask;
          mrb._stageMask |= sasStageGeometryShader;
        }

        if( shaderFlagMask != sasRenderList_ShadowPass )
        {
          mrb._stageMask |= sasStagePixelShader;
        }

        if(bTessellation)
        {
          mrb._stageMask |= ( sasStageHullShader | sasStageDomainShader );
          topology = sasPrimTopology_TriPatch;
        }

        mrb._topology = topology;
        mrb._shaderID = materialShaderID;
        mrb._numPrimitives = subMesh->primitiveCount();
        mrb._rasterizerState = rs;
        mrb._depthState = dss;
        mrb._blendState = bs;

        const smVec4* instPos = instance->wsBoundingSphere();
        smVec3 instToCam = cameraPos - *instPos;
        mrb._distToCamera = smLength3( instToCam );
        mrb.generateSortKey( matType );
      }
      else if ( renderMode == sasRenderMode_BoundingSpheres )
      {
        smMat44 t;
        smEulerAngles r;
        const smVec4* bSphere = instance->wsBoundingSphere();
        smGetXFormTRSMatrix( bSphere->X, bSphere->Y, bSphere->Z, r, bSphere->W, t );
        smVec4 colour( 0.8f, 0.2f, 0.35f, 0.5f );
        sasRenderMng::singletonPtr()->renderBoundingSphere( t, colour );
      }
    }
  }

  radixSortAscending( _numRenderBlobs, _renderBlobs, offsetof( sasModelRenderBlob, _sortKey ), sizeof( uint64_t ) );
}

void sasRenderList::renderList( sasConstantBuffer* constantBuffer )
{
  sasShaderID nullShaderID;
  sasDevice* device = sasDevice::singletonPtr();
  for( unsigned int i = 0; i < _numRenderBlobs; i++ )
  {
    sasModelRenderBlob& mrb = _renderBlobs[ i ];
    device->setRasterizerState( mrb._rasterizerState );
    device->setBlendState( mrb._blendState );
    device->setDepthStencilState( mrb._depthState, mrb._stencilRef );
    device->setVertexBuffer( mrb._vb );
    device->setIndexBuffer( mrb._ib );
    
    setSRVs( mrb, device );

    //VS always present
    device->setVertexShader( mrb._shaderID );
    device->setHullShader( nullShaderID );
    device->setDomainShader( nullShaderID );
    device->setGeometryShader( nullShaderID );
    device->setPixelShader( nullShaderID );
    
    if( mrb._stageMask & sasStageHullShader )
      device->setHullShader( mrb._shaderID );

    if( mrb._stageMask & sasStageDomainShader )
      device->setDomainShader( mrb._shaderID );

    if( mrb._stageMask & sasStageGeometryShader )
      device->setGeometryShader( mrb._shaderID );

    if( mrb._stageMask & sasStagePixelShader )
      device->setPixelShader( mrb._shaderID );

    void* ptr = constantBuffer->map();
    if( ptr )
    {
      sasPerInstanceConstants* cbData = static_cast< sasPerInstanceConstants* >( ptr );
      *cbData = mrb._constants;
      constantBuffer->unmap();
    }
    device->setConstantBuffer( 1, constantBuffer, sasDeviceUnit::VertexShader );   
    device->setConstantBuffer( 1, constantBuffer, sasDeviceUnit::PixelShader );  //instance id is used in PS

    if( mrb._instanceCount == 1 )
        device->drawIndexed( mrb._topology, mrb._numPrimitives );
    else
        device->drawIndexedInstanced( mrb._topology, mrb._numPrimitives, mrb._instanceCount );
  }

  //reset default states
  device->setHullShader( nullShaderID );
  device->setDomainShader( nullShaderID );
  device->setGeometryShader( nullShaderID );
  device->setRasterizerState( sasRasterizerState_Solid );
  device->setBlendState( sasBlendState_Null );
}

void sasRenderList::clearList()
{
  for( unsigned int i = 0; i < _numRenderBlobs; i++ )
  {
    _renderBlobs[ i ].clear();
  }
  _numRenderBlobs = 0;
}

void sasRenderList::setSRVs( sasModelRenderBlob& mrb, sasDevice* device )
{
  for( unsigned int s = 0; s < mrb._numSamplers; s++ )
  {
    device->setSamplerState( mrb._samplers[ s ]._index, mrb._samplers[ s ]._samplerState, mrb._samplers[ s ]._stage );
  }

  for( unsigned int s = 0; s < mrb._numSRVTextures; s++ )
  {
    device->setTexture( mrb._srvTextures[ s ]._index, mrb._srvTextures[ s ]._texture, mrb._srvTextures[ s ]._stage );
  }

  for( unsigned int s = 0; s < mrb._numSRVRTs; s++ )
  {
    device->setTexture( mrb._srvRTs[ s ]._index, mrb._srvRTs[ s ]._rt, mrb._srvRTs[ s ]._stage );
  }

  for( unsigned int s = 0; s < mrb._numSRVDSTs; s++ )
  {
    device->setTexture( mrb._srvDSTs[ s ]._index, mrb._srvDSTs[ s ]._dst, mrb._srvDSTs[ s ]._stage );
  }

  for( unsigned int s = 0; s < mrb._numSRVStructBuffers; s++ )
  {
    device->setStructuredBuffer( mrb._srvStructuredBuffers[ s ]._index, mrb._srvStructuredBuffers[ s ]._sb, mrb._srvStructuredBuffers[ s ]._stage );
  }
}

sasRenderList::sasRenderList()
  : _numRenderBlobs( 0 )
{
}

sasRenderList::~sasRenderList()
{
  clearList();
}

void sasModelRenderBlob::generateSortKey( sasMaterialType::Enum type )
{
  _sortKey = 0;

  uint32_t dist = static_cast< uint32_t >( _distToCamera * 0.5f );                  // 2 unit steps
  _sortKey |= static_cast< uint64_t >( type & 0x7 ) << 60;                          // 3 bits of material type
  _sortKey |= static_cast< uint64_t >( dist & 0xffff ) << 44;                       // 16 bits of distance
  uint32_t shaderHash = sasHasher::fnv_32_buf( &_shaderID, sizeof( _shaderID ) );   // 32 bit of shader type
  _sortKey |= ( shaderHash );

}

void sasModelRenderBlob::pushBlob( sasSamplerStateBlob& blob )
{
  sasASSERT( ( _numSamplers + 1 ) < MAX_NUM_SAMPLERS );
  _samplers[ _numSamplers++ ] = blob;
}

void sasModelRenderBlob::pushBlob( sasTextureBlob& blob )
{
  sasASSERT( ( _numSRVTextures + 1 ) < MAX_NUM_SRV_TEXTURES );
  _srvTextures[ _numSRVTextures++ ] = blob;
}

void sasModelRenderBlob::pushBlob( sasStructuredBufferBlob& blob )
{
  sasASSERT( ( _numSRVStructBuffers + 1 ) < MAX_NUM_SRV_STRUCTBUFFERS );
  _srvStructuredBuffers[ _numSRVStructBuffers++ ] = blob;
}

void sasModelRenderBlob::pushBlob( sasRenderTargetBlob& blob )
{
  sasASSERT( ( _numSRVRTs + 1 ) < MAX_NUM_SRV_RTS );
  _srvRTs[ _numSRVRTs++ ] = blob;
}

void sasModelRenderBlob::pushBlob( sasDepthStencilTargetBlob& blob )
{
  sasASSERT( ( _numSRVDSTs + 1 ) < MAX_NUM_SRV_DSTS );
  _srvDSTs[ _numSRVDSTs++ ] = blob;
}


