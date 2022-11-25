#include "sas_pch.h"
#include "sas_miscresources.h"
#include "sas_vertexbuffer.h"
#include "sas_indexbuffer.h"
#include "sas_vertexformat.h"
#include "sas_constantbuffer.h"
#include "utils/sas_cameramng.h"
#include "utils/sas_camera.h"
#include "sas_texture.h"
#include "sas_device.h"
#include "utils/sas_timer.h"
#include "resource/sas_resourcemng.h"
#include "resource/sas_texture_resource.h"
#include "sas_structuredbuffer.h"
#include "render/sas_settings.h"

struct sasGlobalConstants
{
  smMat44 matView;
  smMat44 matProj;
  smMat44 matViewProj;
  smMat44 matInvViewProj;
  smMat44 matInvProj;
  smMat44 matLastFrameViewProj;
  smMat44 matInvProjViewRescaledPixelPos;
  smMat44 matInvProjViewRescaledUVs;
  smVec4  cameraPositionTime;
  smVec4  backBufferDimensions;
  smVec4  projectionPlanes;
  smVec4  viewSpaceExtents;
};

int getRandomIntInRange( int minRange, int maxRange )
{
  int r = static_cast<int>((float)rand() / (RAND_MAX + 1) * (maxRange - minRange) + minRange);
  return r;
}

float getRandomFloatInRange( float minRange, float maxRange )
{
  float r = ( ( float ) rand() ) / ( float )RAND_MAX;
  return r * ( maxRange - minRange ) + minRange;
}

sasMiscResources::sasMiscResources()
  : _noiseSB( NULL )
{
  createFullScreenQuadVB();
  createUnitCube();
  createUnitSphere();
  createNoiseSB();

  _globalConstants = sasNew sasConstantBuffer( sizeof( sasGlobalConstants ) );
  
  sasTextureMipData noiseMipData;
  noiseMipData.format = sasPixelFormat::RGBA_U8;
  noiseMipData.width = 128;
  noiseMipData.height = 128;
  noiseMipData.dataSize = sizeof( char ) * 4 * noiseMipData.width * noiseMipData.height;
  noiseMipData.data = ( unsigned char* ) sasMalloc( noiseMipData.dataSize );
  noiseMipData.pitch = ( noiseMipData.dataSize / noiseMipData.height );

  unsigned int* noiseData = reinterpret_cast< unsigned int* >( noiseMipData.data );

  for( size_t i = 0; i < noiseMipData.width; i++ )
  {
    for( size_t j = 0; j < noiseMipData.height; j++ )
    {
      unsigned int r = 0;
      r |= getRandomIntInRange( 0, 255 ) << 24;
      r |= getRandomIntInRange( 0, 255 ) << 16;
      r |= getRandomIntInRange( 0, 255 ) << 8;
      r |= getRandomIntInRange( 0, 255 );

      noiseData[ i + j * noiseMipData.width ] = r;
    }
  }

  _noiseTexture = sasNew sasTexture( &noiseMipData, 1, 1 );

  _missingTexture = NULL;
  _selectedTexture = NULL;
}

sasMiscResources::~sasMiscResources()
{
  sasDelete _noiseSB;
  sasDelete _selectedTexture;
  sasDelete _missingTexture;
  sasDelete _globalConstants;
  sasDelete _fullScreenQuadVB;
  sasDelete _unitCubeVB;
  sasDelete _UnitCubeIB;
  sasDelete _noiseTexture;
  sasDelete _unitSphereIB;
  sasDelete _unitSphereVB;
}

void sasMiscResources::loadMiscTextures()
{
  sasASSERT( _missingTexture == NULL );
  std::string texPath = sasResourceMng::singletonPtr()->resourcePath();
  texPath += "misc/invalidTexture.tga";
  _missingTexture = static_cast< sasTextureResource* >( sasResourceMng::singletonPtr()->load( texPath.c_str() ) );

  sasASSERT( _selectedTexture == NULL );
  texPath = sasResourceMng::singletonPtr()->resourcePath();
  texPath += "misc/selectedTexture.tga";
  _selectedTexture = static_cast< sasTextureResource* >( sasResourceMng::singletonPtr()->load( texPath.c_str() ) );
}

void sasMiscResources::createFullScreenQuadVB()
{
  sasVertexFormat fullScreenQuadFmt;
  fullScreenQuadFmt.position = sasVertexFormat::POSITION_3F;
  fullScreenQuadFmt.uv0 = sasVertexFormat::UV_2F;

  _fullScreenQuadVB = sasNew sasVertexBuffer(fullScreenQuadFmt, 4);
  
  const smVec3 pos[4] = {  smVec3(-1.f, 1.f, 0.f),
                           smVec3(-1.f, -1.f, 0.f),
                           smVec3(1.f, 1.f, 0.f),
                           smVec3(1.f, -1.f, 0.f),
                                   };
  const smVec2 uvs[4] = {  smVec2(0.f, 0.f),
                           smVec2(0.f, 1.f),
                           smVec2(1.f, 0.f),
                           smVec2(1.f, 1.f) };

  float* data = (float*)_fullScreenQuadVB->map( false );
  if( data )
  {
    for( size_t i=0; i < 4; i++ )
    {
      memcpy(data, (const float*)&pos[i], sizeof(float) * 3);
      data += 3;
      memcpy(data, (const float*)&uvs[i], sizeof(float) * 2);
      data += 2;
    }
    _fullScreenQuadVB->unmap();
  }

  
}

void sasMiscResources::createUnitCube()
{
    // cube /////////////
    //    v6----- v5
    //   /|      /|
    //  v1------v0|
    //  | |     | |
    //  | |v7---|-|v4
    //  |/      |/
    //  v2------v3

  sasVertexFormat unitCubeFmt;
  unitCubeFmt.position = sasVertexFormat::POSITION_3F;

  const smVec3 pos[] = {  smVec3( 0.5f,  0.5f, -0.5f), // v0
                          smVec3(-0.5f,  0.5f, -0.5f), // v1
                          smVec3(-0.5f, -0.5f, -0.5f), // v2
                          smVec3( 0.5f, -0.5f, -0.5f), // v3
                          smVec3( 0.5f, -0.5f,  0.5f), // v4
                          smVec3( 0.5f,  0.5f,  0.5f), // v5
                          smVec3(-0.5f,  0.5f,  0.5f), // v6
                          smVec3(-0.5f, -0.5f,  0.5f), // v7
                      };

  const size_t nrOfVertices = sizeof(pos) / sizeof(pos[0]);

  const uint16_t indices[] = { 0, 2, 1,   0, 3, 2,  // Front
                               1, 7, 6,   1, 2, 7,  // Left
                               5, 3, 0,   5, 4, 3,  // Right
                               6, 0, 1,   6, 5, 0,  // Top
                               3, 7, 2,   3, 4, 7,  // Bottom
                               5, 6, 7,   5, 7, 4,  // Back
                             };


  const size_t nrOfIndices = sizeof(indices) / sizeof(indices[0]);
  _unitCubeVB = sasNew sasVertexBuffer(unitCubeFmt, nrOfIndices);  

  float* posData = (float*)_unitCubeVB->map( false );
  if( posData != NULL)
  {
    for( size_t i = 0; i < nrOfVertices; i++ )
    {
      memcpy(posData, (const float*)&pos[i], sizeof(float) * 3);
      posData += 3;
    }
    _unitCubeVB->unmap();
  }

   _UnitCubeIB = sasNew sasIndexBuffer( sizeof(indices) / sizeof(indices[0]) );
   
   uint16_t* indexData = _UnitCubeIB->map();
   if( indexData != NULL)
   {
     memcpy(indexData, indices, sizeof(indices));
     _UnitCubeIB->unmap();
   }
}

sasTexture* sasMiscResources::getNoiseTexture() const
{
  return _noiseTexture;
}

void sasMiscResources::updateGlobalShaderConstants( float backbufferWidth, float backbufferHeight, smMat44* view, smMat44* proj, bool initialFrameSet, sasStereoView::Enum eye )
{
  sasCamera* camera = sasCameraMng::singleton().getCurrentCamera();
  sasASSERT( camera );

  smMat44 matView;
  smMat44 matProj;

  if( view == NULL )
  {
    camera->getViewMatrix( matView );
    camera->getProjMatrix( backbufferWidth, backbufferHeight, matProj );
  }
  else
  {
    matView = *view;
    matProj = *proj;
  }
  smMat44 matProjView;
  smMul( matProj, matView, matProjView );

  smMat44 matInvProjView;
  smInverse( matProjView, matInvProjView );
  smMat44 matInvProj;
  smInverse(matProj, matInvProj);

  smMat44 invProjViewRescaleMtx;
  smEulerAngles rot;
  smGetXFormTRSMatrix( -1.f, 1.f, 0, rot, 2.f / sasSettings::singletonPtr()->_frameBufferResolution.width(), 
                        -2.f / sasSettings::singletonPtr()->_frameBufferResolution.height(), 1.f, invProjViewRescaleMtx );
  smMul( matInvProjView, invProjViewRescaleMtx, invProjViewRescaleMtx );

  smMat44 invProjViewRescaleUVsMtx;
  smGetXFormTRSMatrix( -1.f, 1.f, 0, rot, 2.f, -2.f, 1.f, invProjViewRescaleUVsMtx );
  smMul( matInvProjView, invProjViewRescaleUVsMtx, invProjViewRescaleUVsMtx );

  static smMat44 matLastFrameProjViewMono = matProjView;
  static smMat44 matLastFrameProjViewLeft = matProjView;
  static smMat44 matLastFrameProjViewRight = matProjView;

  smMat44 lastFrameVP;
  if( eye == sasStereoView::Mono )
    lastFrameVP = matLastFrameProjViewMono;
  else if( eye == sasStereoView::LeftEye )
    lastFrameVP = matLastFrameProjViewLeft;
  else if( eye == sasStereoView::RightEye )
    lastFrameVP = matLastFrameProjViewRight;


  smVec4 screenSpaceExtents( 1.f, 1.f, 1.f, 1.f );
  smVec4 viewSpaceExtents;
  smMul( matInvProj, screenSpaceExtents, viewSpaceExtents );
  viewSpaceExtents /= viewSpaceExtents.W;


  void* ptr = _globalConstants->map();
  if( ptr )
  {
    sasGlobalConstants* data = reinterpret_cast<sasGlobalConstants*>(ptr);
    data->matView = matView;
    data->matProj = matProj;
    data->matViewProj = matProjView;
    data->matInvViewProj = matInvProjView;
    data->matInvProj = matInvProj;
    data->matLastFrameViewProj = lastFrameVP;
    data->matInvProjViewRescaledPixelPos = invProjViewRescaleMtx;
    data->matInvProjViewRescaledUVs = invProjViewRescaleUVsMtx;
    data->cameraPositionTime = camera->position();
    data->cameraPositionTime.W = sasTimer::singletonPtr()->getTime();
    data->backBufferDimensions = smVec4( backbufferWidth, backbufferHeight, 1.f / backbufferWidth, 1.f / backbufferHeight );
    data->viewSpaceExtents = viewSpaceExtents;
    float zNear = camera->zNear();
    float zFar = camera->zFar();
#ifdef PLATFORM_DX11
    data->projectionPlanes = smVec4( zNear, zFar, zFar / zNear, 1.f - (zFar / zNear) );
#else
    data->projectionPlanes = smVec4( zNear, zFar, zFar / (zFar - zNear), (-zFar * zNear) / (zFar - zNear) );
#endif
    _globalConstants->unmap();
  }
  sasDevice::singletonPtr()->setConstantBuffer( 0, _globalConstants, sasDeviceUnit::VertexShader );
  sasDevice::singletonPtr()->setConstantBuffer( 0, _globalConstants, sasDeviceUnit::HullShader );
  sasDevice::singletonPtr()->setConstantBuffer( 0, _globalConstants, sasDeviceUnit::DomainShader );
  sasDevice::singletonPtr()->setConstantBuffer( 0, _globalConstants, sasDeviceUnit::GeometryShader );
  sasDevice::singletonPtr()->setConstantBuffer( 0, _globalConstants, sasDeviceUnit::PixelShader );
  sasDevice::singletonPtr()->setConstantBuffer( 0, _globalConstants, sasDeviceUnit::ComputeShader );

  if( initialFrameSet )
  {
    if( eye == sasStereoView::Mono )
      matLastFrameProjViewMono = matProjView;
    else if( eye == sasStereoView::LeftEye )
      matLastFrameProjViewLeft = matProjView;
    else if( eye == sasStereoView::RightEye )
      matLastFrameProjViewRight = matProjView;
  }
}

void sasMiscResources::createUnitSphere( sasVertexBuffer** vb, sasIndexBuffer** ib, uint32_t ringCount, uint32_t segmentCount, const sasVertexFormat& vertexFmt )
{
  const size_t numRings = ringCount;
  const size_t numSegments = segmentCount;
  const float radius = 1.f;

  sasASSERT( vertexFmt.position == sasVertexFormat::POSITION_3F );
  sasASSERT( vertexFmt.tangent != sasVertexFormat::TANGENT_3F );
  sasASSERT( vertexFmt.uv0 != sasVertexFormat::UV_2F );
  sasASSERT( vertexFmt.colour != sasVertexFormat::COLOUR_4F );
  sasASSERT( vertexFmt.boneIndices != sasVertexFormat::BONEINDICES );
  sasASSERT( vertexFmt.boneWeights != sasVertexFormat::BONEWEIGHTS );

  size_t numVerts = ( numRings + 1 ) * ( numSegments + 1 );
  size_t numIndices = 6 * numRings * ( numSegments + 1 );

  *vb = sasNew sasVertexBuffer( vertexFmt, numVerts );  
  *ib = sasNew sasIndexBuffer( numIndices );

  float deltaRingAngle = ( smPI / numRings );
  float deltaSegAngle = ( sm2PI / numSegments );
  uint16_t wVerticeIndex = 0 ;
  float* vertexData = static_cast< float* >( ( *vb )->map( false ) );
  uint16_t* indexData = ( *ib )->map();

  // Generate the group of rings for the sphere
  for( size_t ring = 0; ring <= numRings; ring++ ) 
  {
    float r0 = radius * sinf ( ring * deltaRingAngle );
    float y0 = radius * cosf ( ring * deltaRingAngle );

    // Generate the group of segments for the current ring
    for( size_t seg = 0; seg <= numSegments; seg++ ) 
    {
      float x0 = r0 * sinf( seg * deltaSegAngle );
      float z0 = r0 * cosf( seg * deltaSegAngle );

      // Add one vertex to the strip which makes up the sphere
      *vertexData = x0;
      vertexData++;
      *vertexData = y0;
      vertexData++;
      *vertexData = z0;
      vertexData++;

      if( vertexFmt.normal == sasVertexFormat::NORMAL_3F )
      {
        smVec3 n( x0, y0, z0 );
        smNormalize3( n, n );
        *vertexData = n.X;
        vertexData++;
        *vertexData = n.Y;
        vertexData++;
        *vertexData = n.Z;
        vertexData++;
      }

      if( ring != numRings ) 
      {
        // each vertex (except the last) has six indices pointing to it
        *indexData = wVerticeIndex + static_cast< uint16_t >( numSegments ) + 1;
        indexData++;
        *indexData = wVerticeIndex;               
        indexData++;
        *indexData = wVerticeIndex + static_cast< uint16_t >( numSegments );
        indexData++;
        *indexData = wVerticeIndex + static_cast< uint16_t >( numSegments ) + 1;
        indexData++;
        *indexData = wVerticeIndex + 1;
        indexData++;
        *indexData = wVerticeIndex;
        indexData++;
        wVerticeIndex++;
      }
    } 
  }

  // Unlock
  ( *vb )->unmap();
  ( *ib )->unmap();
}

void sasMiscResources::createUnitSphere()
{
  const size_t numRings = 16;
  const size_t numSegments = 16;

  sasVertexFormat unitSphereFmt;
  unitSphereFmt.position = sasVertexFormat::POSITION_3F;

  createUnitSphere( &_unitSphereVB, &_unitSphereIB, numRings, numSegments, unitSphereFmt );
}

sasTexture* sasMiscResources::getMissingTexture() const
{
   return _missingTexture->texture();
}

sasTexture* sasMiscResources::getSelectedTexture() const
{
  return _selectedTexture->texture();
}

void sasMiscResources::createNoiseSB()
{
  _noiseSB = sasNew sasStructuredBuffer( 1024 * sizeof( float ), sizeof( float ), true, false, false );
  float* data = static_cast< float* >( _noiseSB->map() );
  for( unsigned int i = 0; i < 1024; i++ )
    data[ i ] = getRandomFloatInRange( 0, 1.f );
  _noiseSB->unmap();
}




