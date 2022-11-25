#include "sas_pch.h"
#include "sas_model_loader.h"
#include "sas_model_resource.h"
#include "sas_resourcemng.h"
#include "sasMaths.h"
#include "render/sas_vertexbuffer.h"
#include "render/sas_vertexformat.h"
#include "render/sas_indexbuffer.h"
#include "render/sas_instance.h"
#include "render/sas_geometry.h"
#include "render/sas_mesh.h"
#include "render/sas_material.h"
#include "render/sas_miscresources.h"
#include "animation/sas_skeleton.h"
#include "animation/sas_animation.h"
#include "sas_texture_resource.h"
#include "utils/sas_camera.h"
#include <importer.hpp>
#include <postprocess.h>
#include <scene.h>
#include <mesh.h>
#include "utils/sas_file.h"
#include "utils/sas_virtual_file.h"


/*
  loading functions for sasModel data format
*/
bool loadVertexBuffer( sasVirtualFile& vFile, sasVertexBuffer** vb );
bool loadIndexBuffer( sasVirtualFile& vFile, sasIndexBuffer** ib );
bool loadSubmesh( sasVirtualFile& vFile, sasSubMesh** sm, sasModelResourceData& mrd );
bool loadMesh( sasVirtualFile& vFile, sasMesh** m, sasModelResourceData& mrd );
bool loadGeo( sasVirtualFile& vFile, sasGeometry** g, sasModelResourceData& mrd );
bool loadInstances( sasVirtualFile& vFile, sasStagingInstance** inst, sasModelResourceData& mrd );
bool loadSkeleton( sasVirtualFile& vFile, sasSkeleton** s, sasModelResourceData& mrd );
bool loadSkeletonHierarchy( sasVirtualFile& vFile, sasSkeletonHierarchy* sh, sasModelResourceData& mrd );
bool loadMaterial( sasVirtualFile& vFile, sasMaterial** m, sasModelResourceData& mrd );
bool loadAnimation( sasVirtualFile& vFile, sasAnimation** a, sasModelResourceData& mrd );
bool loadModelResource( sasVirtualFile& vFile, sasModelResourceData* mr );
sasResource* createBlobModelResource( sasStringId id );


void sasModelRegisterLoader()
{
  sasModelLoader* loader = sasNew sasModelLoader();
  sasResourceMng::singleton().registerLoader( "sasModel", loader );
}


sasResource* sasModelLoader::load( const char* path )
{
  smVec3 scale( 1.f );
  smVec3 translation( 0.f );
  std::string name = "unnamed_resource";
  return load( path, name.c_str(), scale, translation );
}

sasResource* sasModelLoader::load( const char* path, const char* name )
{
  smVec3 scale( 1.f );
  smVec3 translation( 0.f );
  return load( path, name, scale, translation );
}

sasResource* sasModelLoader::load( const char* path, const char* name, const smVec3& scale, const smVec3& translation )
{
  sasVirtualFile vFile;
  
  {
    sasFile mFile;
    if( !mFile.open( path, sasFile::Read ) )
    {
      sasDebugOutput( "sasModelLoader::load - failed to open file: %s\n", path );
      return nullptr;
    }

    vFile.open( mFile.getSize() );
    size_t sizeCopied = mFile.read( &vFile, mFile.getSize() );
    sasASSERT( sizeCopied == mFile.getSize() );
  }
  

  sasModelResourceData buildContext;
  buildContext.skeletonHierarchy = NULL;

  sasStringId nameHash = sasStringId::build( name );

  bool r = loadModelResource( vFile, &buildContext );

  if( !r )
  {
    sasDebugOutput( "sasModelLoader::load - failed to load resources for model %s\n", path );
    return nullptr;
  }

  sasModelResource* resource = sasNew sasModelResource( buildContext );
  return resource;
}

bool loadModelResource( sasVirtualFile& vFile, sasModelResourceData* mrd )
{
  static const uint32_t kModelResourceVersion = 5;
  uint32_t modelVersion;
  vFile.read( modelVersion );

  if( modelVersion != kModelResourceVersion )
  {
    return false;
  }

  bool r = true;

  //materials
  size_t numMaterials = 0;
  vFile.read( numMaterials );
  for( size_t i = 0; i < numMaterials; i++ )
  {
    sasMaterial* m = nullptr;
    r &= loadMaterial( vFile, &m, *mrd );
    mrd->materials.push_back( m );
  }

  //vbs
  size_t numVbs = 0;
  vFile.read( numVbs );
  for( size_t i = 0; i < numVbs; i++ )
  {
    sasVertexBuffer* vb = nullptr;
    r &= loadVertexBuffer( vFile, &vb );
    mrd->vertexBuffers.push_back( vb );
  }

  //ibs
  size_t numIbs = 0;
  vFile.read( numIbs );
  for( size_t i = 0; i < numIbs; i++ )
  {
    sasIndexBuffer* ib = nullptr;
    r &=loadIndexBuffer( vFile, &ib );
    mrd->indexBuffers.push_back( ib );
  }

  //submeshes
  size_t numSubmeshes = 0;
  vFile.read( numSubmeshes );
  for( size_t i = 0; i < numSubmeshes; i++ )
  {
    sasSubMesh* sm = nullptr;
    r &= loadSubmesh( vFile, &sm, *mrd );
    mrd->subMeshes.push_back( sm );
  }

  //meshes
  size_t numMeshes = 0;
  vFile.read( numMeshes );
  for( size_t i = 0; i < numMeshes; i++ )
  {
    sasMesh* m = nullptr;
    r &= loadMesh( vFile, &m, *mrd );
    mrd->meshes.push_back( m );
  }

  //geometries
  size_t numGeos = 0;
  vFile.read( numGeos );
  for( size_t i = 0; i < numGeos; i++ )
  {
    sasGeometry* g = nullptr;
    r &= loadGeo( vFile, &g, *mrd );
    mrd->geometries.push_back( g );
  }

  //skeleton hierarchy
  uint32_t hasSkeletonHierarchy = 0;
  vFile.read( hasSkeletonHierarchy );
  if( hasSkeletonHierarchy )
  {
    mrd->skeletonHierarchy = sasNew sasSkeletonHierarchy();
    r &= loadSkeletonHierarchy( vFile, mrd->skeletonHierarchy, *mrd );
  }

  //skeletons
  size_t numSkeletons = 0;
  vFile.read( numSkeletons );
  for( size_t i = 0; i < numSkeletons; i++ )
  {
    sasSkeleton* s = nullptr;
    uint32_t isPresent;
    vFile.read( isPresent );
    if( isPresent )
    {
      r &= loadSkeleton( vFile, &s, *mrd );
    }
    mrd->skeletons.push_back( s );
  }

  //animations
  size_t numAnimations = 0;
  vFile.read( numAnimations );
  for( size_t i = 0; i < numAnimations; i++ )
  {
    sasAnimation* a = nullptr;
    r &= loadAnimation( vFile, &a, *mrd );
    mrd->compatibleAnimations.push_back( a );
  }

  //instances
  size_t numInstances = 0;
  vFile.read( numInstances );
  for( size_t i = 0; i < numInstances; i++ )
  {
    sasStagingInstance* inst = nullptr;
    r &= loadInstances( vFile, &inst, *mrd );
    mrd->instances.push_back( inst );
  }

  //should never happen, file existence and version checks have happened previously, so assert on result
  sasASSERT( r );

  return r;
}

bool loadVertexBuffer( sasVirtualFile& vFile, sasVertexBuffer** vb )
{
  sasASSERT( *vb == nullptr );
  uint32_t version;
  size_t nVerts;
  size_t size;
  sasVertexFormat fmt;
  
  //ignore version for vb, we only use version for parent model file. This is there just in case we need it later
  vFile.read( version );
  vFile.read( fmt.flags );
  vFile.read( nVerts );
  vFile.read( size );

  *vb = sasNew sasVertexBuffer( fmt, nVerts );

  uint8_t* vbData = reinterpret_cast< uint8_t* >( ( *vb )->map( false ) );
  vFile.read( vbData, size );
  ( *vb )->unmap();

  return true;
}

bool loadIndexBuffer( sasVirtualFile& vFile, sasIndexBuffer** ib )
{
  sasASSERT( *ib == nullptr );
  uint32_t version;
  size_t nIndices;
  size_t size;

  //ignore version, we only use version for parent model file. This is there just in case we need it later
  vFile.read( version );
  vFile.read( nIndices );
  vFile.read( size );

  *ib = sasNew sasIndexBuffer( nIndices );

  uint8_t* ibData = reinterpret_cast< uint8_t* >( ( *ib )->map() );
  vFile.read( ibData, size );
  ( *ib )->unmap();

  return true;
}

bool loadSubmesh( sasVirtualFile& vFile, sasSubMesh** sm, sasModelResourceData& mrd )
{
  sasASSERT( *sm == nullptr );
  uint32_t version;
  uint32_t vbIndex;
  uint32_t ibIndex;
  size_t numPrims;
  size_t firstIndex;
  uint32_t materialIndex;
  uint32_t isSkinned;
  float boundingSphere[ 4 ];

  vFile.read( version );
  vFile.read( vbIndex );
  vFile.read( ibIndex );
  vFile.read( numPrims );
  vFile.read( firstIndex );
  vFile.read( materialIndex );
  vFile.read( isSkinned );
  for( unsigned int i = 0; i < 4; i++ )
    vFile.read( boundingSphere[ i ] );

  sasSubMeshSetup setup;
  setup.boundingSphere = smVec4( boundingSphere[ 0 ], boundingSphere[ 1 ], boundingSphere[ 2 ], boundingSphere[ 3 ] );
  setup.firstIndex = firstIndex;
  setup.ib = mrd.indexBuffers[ ibIndex ];
  setup.vb = mrd.vertexBuffers[ vbIndex ];
  setup.isSkinned = ( isSkinned == TRUE );
  setup.material = mrd.materials[ materialIndex ];
  setup.nPrims = numPrims;
  *sm = sasNew sasSubMesh( setup );

  return true;
}

bool loadMesh( sasVirtualFile& vFile, sasMesh** m, sasModelResourceData& mrd )
{
  sasASSERT( *m == nullptr );
  uint32_t version;
  size_t submeshCount;
  float boundingSphere[ 4 ];

  vFile.read( version );
  vFile.read( submeshCount );

  uint32_t* submeshIndices = sasSTACK_ALLOC( uint32_t, submeshCount );
  for( uint32_t i = 0; i < submeshCount; i++ )
    vFile.read( submeshIndices[ i ] );

  for( unsigned int i = 0; i < 4; i++ )
    vFile.read( boundingSphere[ i ] );

  sasSubMesh** sms = sasSTACK_ALLOC( sasSubMesh*, submeshCount );
  for( uint32_t i = 0; i < submeshCount; i++ )
    sms[ i ] = mrd.subMeshes[ submeshIndices[ i ] ];

  *m = sasNew sasMesh( sms, submeshCount );
  
  return true;
}


bool loadGeo( sasVirtualFile& vFile, sasGeometry** g, sasModelResourceData& mrd )
{
  sasASSERT( *g == nullptr );
  uint32_t version;
  uint32_t numLods;
  
  vFile.read( version );
  vFile.read( numLods );

  float* distances = sasSTACK_ALLOC( float, numLods );
  uint32_t* meshIndices = sasSTACK_ALLOC( uint32_t, numLods );

  for( uint32_t i = 0; i < numLods; i++ )
  {
    vFile.read( distances[ i ] );
    vFile.read( meshIndices[ i ] );
  }

  sasGeometryLOD* lods = sasSTACK_ALLOC( sasGeometryLOD, numLods );
  for( uint32_t i = 0; i < numLods; i++ )
  {
    lods[ i ].distance = distances[ i ];
    lods[ i ].mesh = mrd.meshes[ meshIndices[ i ] ];
  }

  *g = sasNew sasGeometry( lods, numLods );
  
  return true;
}

bool loadInstances( sasVirtualFile& vFile, sasStagingInstance** inst, sasModelResourceData& mrd )
{
  sasASSERT( *inst == nullptr );
  uint32_t version;
  uint32_t geoIndex;
  float xform[ 4 ][ 4 ];

  vFile.read( version );
  vFile.read( geoIndex );
  for( uint32_t i = 0; i < 4; i++ )
    vFile.read( xform[ 0 ][ i ] );
  for( uint32_t i = 0; i < 4; i++ )
    vFile.read( xform[ 1 ][ i ] );
  for( uint32_t i = 0; i < 4; i++ )
    vFile.read( xform[ 2 ][ i ] );
  for( uint32_t i = 0; i < 4; i++ )
    vFile.read( xform[ 3 ][ i ] );

  *inst = sasNew sasStagingInstance( mrd.geometries[ geoIndex ] );

  smMat44 transform;
  transform.L0 = smVec4( xform[ 0 ][ 0 ], xform[ 0 ][ 1 ], xform[ 0 ][ 2 ], xform[ 0 ][ 3 ] );
  transform.L1 = smVec4( xform[ 1 ][ 0 ], xform[ 1 ][ 1 ], xform[ 1 ][ 2 ], xform[ 1 ][ 3 ] );
  transform.L2 = smVec4( xform[ 2 ][ 0 ], xform[ 2 ][ 1 ], xform[ 2 ][ 2 ], xform[ 2 ][ 3 ] );
  transform.L3 = smVec4( xform[ 3 ][ 0 ], xform[ 3 ][ 1 ], xform[ 3 ][ 2 ], xform[ 3 ][ 3 ] );
  ( *inst )->setTransform( transform );

  return true;
}

bool loadSkeleton( sasVirtualFile& vFile, sasSkeleton** s, sasModelResourceData& mrd )
{
  sasASSERT( *s == nullptr );
  uint32_t version;
  uint32_t numBones;
  std::string name;

  vFile.read( version );
  vFile.readStr( name );
  vFile.read( numBones );

  *s = sasNew sasSkeleton( numBones );

  ( *s )->_skeletonId = sasStringId::build( name.c_str() );
  
  for( uint32_t i = 0; i < numBones; i++ )
  {
    vFile.readStr( name );
    ( *s )->_boneIdArray[ i ] = sasStringId::build( name.c_str() );
  }

  for( uint32_t i = 0; i < numBones; i++ )
  {
    vFile.readStr( name );
    ( *s )->_parentBoneIdArray[ i ] = sasStringId::build( name.c_str() );
  }

  for( uint32_t i = 0; i < numBones; i++ )
  {
    for( uint32_t j = 0; j < 4; j++ )
      vFile.read( ( *s )->_boneMatrices[ i ].L0[ j ] );

    for( uint32_t j = 0; j < 4; j++ )
      vFile.read( ( *s )->_boneMatrices[ i ].L1[ j ] );

    for( uint32_t j = 0; j < 4; j++ )
      vFile.read( ( *s )->_boneMatrices[ i ].L2[ j ] );

    for( uint32_t j = 0; j < 4; j++ )
      vFile.read( ( *s )->_boneMatrices[ i ].L3[ j ] );
  }

  ( *s )->_skeletonHierarchy = mrd.skeletonHierarchy;
  return true;
}

bool loadSkeletonHierarchy( sasVirtualFile& vFile, sasSkeletonHierarchy* sh, sasModelResourceData& mrd )
{
  sasASSERT( sh != nullptr );

  uint32_t rootNodeIndex;
  uint32_t numNodes;
  vFile.read( rootNodeIndex );
  vFile.read( numNodes );

  sh->_numNodes = numNodes;

  uint32_t* nodeChildrenIndices = ( uint32_t* )sasMalloc( sizeof( uint32_t ) * sasSkeletonNode::kMaxNumChildrenNodes * numNodes );
  uint32_t* parentIndices = ( uint32_t* )sasMalloc( sizeof( uint32_t ) * numNodes );

  float xform[ 4 ][ 4 ];

  //read all nodes, before resolving indices
  std::string name;
  for( uint32_t n = 0; n < numNodes; n++ )
  {
    sasSkeletonNode* sn = &( sh->_skeletonNodes[ n ] );

    vFile.readStr( name );
    sn->_name = sasStringId::build( name.c_str() );
    vFile.read( sn->_boneIndex );
    vFile.read( parentIndices[ n ] );
    vFile.read( sn->_numChildren );

    for( uint32_t c = 0; c < sn->_numChildren; c++ )
    {
      vFile.read( nodeChildrenIndices[ n * sasSkeletonNode::kMaxNumChildrenNodes + c ] );
    }

    for( uint32_t j = 0; j < 4; j++ )
      vFile.read( xform[ 0 ][ j ] );
    for( uint32_t j = 0; j < 4; j++ )
      vFile.read( xform[ 1 ][ j ] );
    for( uint32_t j = 0; j < 4; j++ )
      vFile.read( xform[ 2 ][ j ] );
    for( uint32_t j = 0; j < 4; j++ )
      vFile.read( xform[ 3 ][ j ] );

    sn->_transform.L0 = smVec4( xform[ 0 ][ 0 ], xform[ 0 ][ 1 ], xform[ 0 ][ 2 ], xform[ 0 ][ 3 ] );
    sn->_transform.L1 = smVec4( xform[ 1 ][ 0 ], xform[ 1 ][ 1 ], xform[ 1 ][ 2 ], xform[ 1 ][ 3 ] );
    sn->_transform.L2 = smVec4( xform[ 2 ][ 0 ], xform[ 2 ][ 1 ], xform[ 2 ][ 2 ], xform[ 2 ][ 3 ] );
    sn->_transform.L3 = smVec4( xform[ 3 ][ 0 ], xform[ 3 ][ 1 ], xform[ 3 ][ 2 ], xform[ 3 ][ 3 ] );
  }

  //resolve indices
  sh->_rootNode = &( sh->_skeletonNodes[ rootNodeIndex ] );
  for( uint32_t n = 0; n < numNodes; n++ )
  {
    sasSkeletonNode* sn = &( sh->_skeletonNodes[ n ] );
    uint32_t parentIndex = parentIndices[ n ];
    sn->_parent = ( parentIndex == 0xffffffff ) ? nullptr : &( sh->_skeletonNodes[ parentIndex ] );
    for( uint32_t c = 0; c < sn->_numChildren; c++ )
    {
      uint32_t childIndex = nodeChildrenIndices[ n * sasSkeletonNode::kMaxNumChildrenNodes + c ];
      sn->_children[ c ] = ( childIndex == 0xffffffff ) ? nullptr : &( sh->_skeletonNodes[ childIndex ] );
    }
  }

  sasFree( parentIndices );
  sasFree( nodeChildrenIndices );
  return true;
}

bool loadMaterial( sasVirtualFile& vFile, sasMaterial** m, sasModelResourceData& mrd )
{
  sasASSERT( *m == nullptr );
  uint32_t version;
  uint32_t numLayers;
  sasMaterialType::Enum matType;

  vFile.read( version );
  vFile.read( numLayers );
  vFile.read( matType );

  const uint32_t kMaxMaterialLayers = 16;
  sasMaterialLayer layers[ kMaxMaterialLayers ];
  for( uint32_t i = 0; i < numLayers; i++ )
  {
    sasTextureId texId;
    layers[ i ].texture = nullptr;
    vFile.read( layers[ i ].type );
    vFile.read( texId );
    
    if( texId != (sasTextureId)0 )
    {
      std::string texPath;
      vFile.readStr( texPath );
      //layers[ i ].texture = sasTextureMng::singletonPtr()->getTexture( texIds[ i ] );
      //if( !layers[ i ].texture )
        layers[ i ].texture = ( sasTextureResource* )sasResourceMng::singleton().load( texPath.c_str() );
    }
  }

  //default material if none is present
  if( numLayers == 0 )
  {
    layers[ 0 ] = sasMaterialLayer();
    numLayers = 1;
  }

  *m = sasNew sasMaterial( layers, numLayers );

  return true;
}

bool loadAnimation( sasVirtualFile& vFile, sasAnimation** a, sasModelResourceData& mrd )
{
  sasASSERT( *a == nullptr );
  uint32_t version;
  std::string name;
  double ticks;
  double tickerPerSecond;
  uint32_t numChannels;

  vFile.read( version );
  vFile.readStr( name );
  vFile.read( ticks );
  vFile.read( tickerPerSecond );
  vFile.read( numChannels );

  sasStringId nameId = sasStringId::build( name.c_str() );

  *a = sasNew sasAnimation( nameId, ticks, tickerPerSecond, numChannels );

  for( uint32_t ac = 0; ac < numChannels; ac++ )
  {
    sasAnimationChannel* animChannel = &( ( *a )->_channels[ ac ] );
    vFile.readStr( name );
    animChannel->_boneId = sasStringId::build( name.c_str() );
    
    //position frames
    vFile.read( animChannel->_numPositionFrames );
    for( uint32_t p = 0; p < animChannel->_numPositionFrames; p++ )
    {
      vFile.read( animChannel->_positionFrames[ p ]._time );
      vFile.read( animChannel->_positionFrames[ p ]._position.X );
      vFile.read( animChannel->_positionFrames[ p ]._position.Y );
      vFile.read( animChannel->_positionFrames[ p ]._position.Z );
    }

    //rotation frames
    vFile.read( animChannel->_numRotationFrames );
    for( uint32_t r = 0; r < animChannel->_numRotationFrames; r++ )
    {
      vFile.read( animChannel->_rotationFrames[ r ]._time );
      vFile.read( animChannel->_rotationFrames[ r ]._rotation.x );
      vFile.read( animChannel->_rotationFrames[ r ]._rotation.y );
      vFile.read( animChannel->_rotationFrames[ r ]._rotation.z );
      vFile.read( animChannel->_rotationFrames[ r ]._rotation.w );
    }

    //scaling frames
    vFile.read( animChannel->_numScalingFrames );
    for( uint32_t s = 0; s < animChannel->_numScalingFrames; s++ )
    {
      vFile.read( animChannel->_scalingFrames[ s ]._time );
      vFile.read( animChannel->_scalingFrames[ s ]._scale.X );
      vFile.read( animChannel->_scalingFrames[ s ]._scale.Y );
      vFile.read( animChannel->_scalingFrames[ s ]._scale.Z );
    }
  }

  sasResourceMng::singleton().registerAnimation( *a );

  return true;
}


sasResource* createBlobModelResource( sasStringId id )
{
  sasModelResourceData buildContext;

  //material
  sasMaterialLayer dummyLayer;
  sasMaterial* material = sasNew sasMaterial( &dummyLayer, 1 );
  buildContext.materials.push_back( material );

  //vbs / ibs
  sasVertexBuffer* vb = nullptr;
  sasIndexBuffer* ib = nullptr;
  sasVertexFormat vertFmt;
  vertFmt.position = sasVertexFormat::POSITION_3F;
  vertFmt.normal = sasVertexFormat::NORMAL_3F;
  sasMiscResources::singleton().createUnitSphere( &vb, &ib, 8, 8, vertFmt );
  buildContext.vertexBuffers.push_back( vb );
  buildContext.indexBuffers.push_back( ib );

  //submeshes
  sasSubMeshSetup subMeshSetup;
  subMeshSetup.vb = vb;
  subMeshSetup.ib = ib;
  subMeshSetup.isSkinned = false;
  subMeshSetup.firstIndex = 0;
  subMeshSetup.boundingSphere = smVec4( 0.f, 0.f, 0.f, 1.f );
  subMeshSetup.material = material;
  subMeshSetup.nPrims = ib->indicesCount() / 3;
  sasSubMesh* sm = sasNew sasSubMesh( subMeshSetup );
  buildContext.subMeshes.push_back( sm );

  //meshes
  sasMesh* mesh = sasNew sasMesh( &sm, 1 );
  buildContext.meshes.push_back( mesh );

  //geometries
  sasGeometryLOD geoLod;
  geoLod.distance = 0.f;
  geoLod.mesh = mesh;
  sasGeometry* geo = sasNew sasGeometry( &geoLod, 1 );
  buildContext.geometries.push_back( geo );

  //instances
  sasStagingInstance* instance = sasNew sasStagingInstance( geo );
  smMat44 instXfm;
  instXfm.setIdentity();
  instance->setTransform( instXfm );
  buildContext.instances.push_back( instance );

  buildContext.skeletonHierarchy = NULL;

  sasModelResource* resource = sasNew sasModelResource( buildContext );
  resource->setName( id );
  return resource;
}



