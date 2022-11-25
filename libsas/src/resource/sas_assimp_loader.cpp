#include "sas_pch.h"
#include "sas_assimp_loader.h"
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
#include "utils/sas_path.h"

/*
  loading functions for assimp data
*/
static void extractMeshes( const aiScene*, sasModelResourceData& buildContext );
static void extractSkeletonMappings( const aiScene*, sasModelResourceData& buildContext );
static void extractCameras( const aiScene*, sasModelResourceData& buildContext );
static void extractSkeletonMapping( const aiMesh* mesh, sasModelResourceData& buildContext );
static void extractMaterials( const aiScene*, sasModelResourceData& buildContext, const char* path );
static sasSubMesh* extractMesh( const aiMesh*, const sasSkeletonMapping* skeletonMapping, sasModelResourceData& buildContext );
static sasCamera* extractCamera( const aiCamera*, sasModelResourceData& buildContext );
static sasMaterial* extractMaterial( const aiMaterial*, sasModelResourceData& buildContext, const char* path );
static sasVertexBuffer* extractMeshVertices( const aiMesh* mesh, const sasSkeletonMapping* skeletonMapping, smVec4* boundingSphere = NULL );
static sasIndexBuffer*  extractMeshIndices( const aiMesh* mesh );
static void extractInstanceNodes( const aiScene* scene, const aiNode* node, sasSkeletonNode* parent, sasModelResourceData& buildContext );
static void extractSkeletonHierarchyNodes( const aiScene* scene, const aiNode* node, sasSkeletonNode* parent, sasModelResourceData& buildContext );
static void extractNodes( const aiScene*, const aiNode* node, sasSkeletonNode* parent, sasModelResourceData& buildContext, bool extractSkeletonHierarchy );
static sasSkeletonNode*  extractNode( const aiScene* ,const aiNode*, sasSkeletonNode* parent, sasModelResourceData& buildContext, bool extractSkeletonHierarchy );
static void extractInstance( const aiScene* , const aiNode*, sasModelResourceData& buildContext );
static void extractAnimations( const char* modelName, const aiScene* scene, sasModelResourceData& buildContext );
static void extractAnimation( const char* modelName, const aiAnimation* animation, sasModelResourceData& buildContext );
static smAABB computeBoundingBox( sasGeometryLOD* lods, size_t nLods );
static smAABB computeBoundingBox( sasMesh* mesh );
static smAABB computeBoundingBox( sasSubMesh* mesh );

/*
  physics data
*/
struct offsetBox
{
  smAABB  aabb;
  smMat44 xform;
};

bool extractPhysicsBoxList( sasModelResourceData& mrd, offsetBox** boxList, uint32_t& numBoxes );
bool serializePhysicsData( sasVirtualFile& vFile, offsetBox* boxList, uint32_t numBoxes );

/*
  saving functions for assimp data to sasModel format
*/
bool serializeVertexBuffer( sasVirtualFile& vFile, const sasVertexBuffer* vb );
bool serializeIndexBuffer( sasVirtualFile& vFile, const sasIndexBuffer* ib );
bool serializeSubmesh( sasVirtualFile& vFile, const sasSubMesh* sm, const sasModelResourceData& mrd );
bool serializeMesh( sasVirtualFile& vFile, const sasMesh* m, const sasModelResourceData& mrd );
bool serializeGeo( sasVirtualFile& vFile, const sasGeometry* g, const sasModelResourceData& mrd );
bool serializeInstances( sasVirtualFile& vFile, const sasStagingInstance* inst, const sasModelResourceData& mrd );
bool serializeModelResource( sasVirtualFile& vFile, const sasModelResource* mr );




void sasAssimpRegisterLoader()
{
  sasAssimpLoader* loader = sasNew sasAssimpLoader;
  sasResourceMng::singleton().registerLoader( "obj", loader );
  sasResourceMng::singleton().registerLoader( "dae", loader );
  sasResourceMng::singleton().registerLoader( "md5mesh", loader );
}

void getMinVector(aiVector3D& v1, aiVector3D& v2, aiVector3D& r)
{
  float x = smMin(v1.x, v2.x);
  float y = smMin(v1.y, v2.y);
  float z = smMin(v1.x, v2.z);

  r = aiVector3D(x, y, z);
}

void getMaxVector(aiVector3D& v1, aiVector3D& v2, aiVector3D& r)
{
  float x = smMax(v1.x, v2.x);
  float y = smMax(v1.y, v2.y);
  float z = smMax(v1.x, v2.z);

  r = aiVector3D(x, y, z);
}

void writeAnimDescFile( const char* path, const sasModelResourceData& resource )
{
  if( resource.compatibleAnimations.size() == 0 )
    return;

  std::string filename = path;
  filename += ".sasAnimFile";
  
  sasFile aFile;
  if( aFile.open( filename.c_str(), sasFile::Read ) )
  {
    //already there, no need to write it out
    return;
  }
  
  if( !aFile.open( filename.c_str(), sasFile::Write ) )
  {
    sasDebugOutput( "writeAnimDescFile: Filed to open file for write %s...\n", filename.c_str() );
    return;
  }

  aFile.write( resource.compatibleAnimations.size() );

  for( size_t i = 0; i < resource.compatibleAnimations.size(); i++ )
  {
    sasAnimation* anim = resource.compatibleAnimations[ i ];
    sasStringId animName = anim->_animationId;
    unsigned int nameLen = strlen( animName.string() );
    aFile.write( nameLen );
    aFile.write( reinterpret_cast< const uint8_t* >( animName.string() ), nameLen );

    float ticksPerSecond = ( float )( anim->_ticksPerSecond != 0.f ? anim->_ticksPerSecond : 25.f );
    float animDuration = static_cast< float >( anim->_animationTicksDuration ) / ticksPerSecond;
    aFile.write( animDuration );
  }

  sasDebugOutput( "writeAnimDescFile: generated animation description file %s ( %d anims )\n", filename.c_str(), static_cast< uint32_t >( resource.compatibleAnimations.size() ) );
}

sasResource* sasAssimpLoader::load( const char* path )
{
  smVec3 scale( 1.f );
  smVec3 translation( 0.f );
  std::string name = "unnamed_resource";
  return load( path, name.c_str(), scale, translation );
}

sasResource* sasAssimpLoader::load( const char* path, const char* name )
{
  smVec3 scale( 1.f );
  smVec3 translation( 0.f );
  return load( path, name, scale, translation );
}

sasResource* sasAssimpLoader::load( const char* path, const char* name, const smVec3& scale, const smVec3& translation )
{
  Assimp::Importer importer;
  
  const aiScene* scene = importer.ReadFile( path, 
    aiProcess_GenSmoothNormals       |
    aiProcess_Triangulate            |
    aiProcess_JoinIdenticalVertices  |
    aiProcess_ImproveCacheLocality   |
    aiProcess_CalcTangentSpace );

  if( !scene)
  {    
    sasDebugOutput( "Failed to find model file: %s\n", path );
    sasASSERT( false );
    return 0;
  } 

  sasModelResourceData buildContext;
  sasStringId nameHash = sasStringId::build( name );

  buildContext.skeletonHierarchy = NULL;
  if( scene->HasAnimations() )
  {
    buildContext.skeletonHierarchy = sasNew sasSkeletonHierarchy();
  }

  extractCameras( scene, buildContext );  
  extractMaterials( scene, buildContext, path );
  extractSkeletonHierarchyNodes( scene, scene->mRootNode, NULL, buildContext );
  extractSkeletonMappings( scene, buildContext );
  extractMeshes( scene, buildContext );     
  extractInstanceNodes( scene, scene->mRootNode, NULL, buildContext );
  extractAnimations( name, scene, buildContext );

  sasModelResource* resource = sasNew sasModelResource( buildContext );

  writeAnimDescFile( path, buildContext );

  resource->setName( nameHash );

  //resource is loaded, now write it out in engine fmt rather than collada fmt for faster loading next time
  sasVirtualFile vFile;
  vFile.open( 5 * 1024 * 1024 );
  serializeModelResource( vFile, resource );
  
  //now flush to disk
  char newPath[ MAX_PATH ];
  sasPathRemoveExtension( path, newPath, MAX_PATH );
  std::string sasModelPath = newPath;
  sasModelPath += ".sasModel";
  sasFile diskFile;
  if( !diskFile.open( sasModelPath.c_str(), sasFile::Write ) )
  {
    sasDebugOutput( "Failed to open file %s for write, could not save model: %s to internal format.\n", sasModelPath.c_str(), name );
  }
  else
  {
    diskFile.write( vFile.data(), vFile.currentSize() );
    sasDebugOutput( "Saved model in sas format: %s. (size: %d)\n", sasModelPath.c_str(), vFile.currentSize() );
  }
  
  //physics data
  offsetBox* boxList = nullptr;
  uint32_t numBoxes = 0;
  extractPhysicsBoxList( buildContext, &boxList, numBoxes );
  sasVirtualFile physicsVFile;
  physicsVFile.open( 1024 );
  serializePhysicsData( physicsVFile, boxList, numBoxes );
  sasFree( boxList );

  //flush to disk
  std::string sasPhysicsPath = newPath;
  sasPhysicsPath += ".sasPhysics";
  sasFile diskPhysicsFile;
  if( !diskPhysicsFile.open( sasPhysicsPath.c_str(), sasFile::Write ) )
  {
    sasDebugOutput( "Failed to open file %s for write, could not save model physics data: %s to internal format.\n", sasPhysicsPath.c_str(), name );
  }
  else
  {
    diskPhysicsFile.write( physicsVFile.data(), physicsVFile.currentSize() );
    sasDebugOutput( "Saved model physics data in sas format: %s. (size: %d)\n", sasPhysicsPath.c_str(), physicsVFile.currentSize() );
  }

  return resource;
}

void extractSkeletonMappings( const aiScene* scene, sasModelResourceData& buildContext )
{
  for( size_t i=0; i<scene->mNumMeshes; i++ )
  {
    aiMesh* m = scene->mMeshes[i];
    extractSkeletonMapping( m, buildContext );
  }
}

void extractMeshes( const aiScene* scene, sasModelResourceData& buildContext )
{  
  for( size_t i=0; i<scene->mNumMeshes; i++ )
  {
    aiMesh* m = scene->mMeshes[i];
    sasSubMesh* subMesh = extractMesh( m, buildContext.skeletonMappings[ i ], buildContext );
    sasASSERT( subMesh );
    buildContext.subMeshes.push_back( subMesh );
  }
}

void extractCameras( const aiScene* scene, sasModelResourceData& buildContext )
{  
  for( size_t i=0; i<scene->mNumCameras; i++ )
  {
    aiCamera* m = scene->mCameras[i];
    sasCamera* camera = extractCamera( m, buildContext );
    sasASSERT( camera );
    buildContext.cameras.push_back( camera );
  }
}

void extractMaterials( const aiScene* scene, sasModelResourceData& buildContext, const char* path )
{
  for( size_t i=0; i<scene->mNumMaterials; i++ )
  {
    aiMaterial* m = scene->mMaterials[i];
    sasMaterial* material = extractMaterial( m, buildContext, path );
    sasASSERT( material );
    buildContext.materials.push_back( material );
  }
}

void extractSkeletonMapping( const aiMesh* mesh, sasModelResourceData& buildContext )
{
  if( !mesh->HasBones() )
  {
    buildContext.skeletonMappings.push_back( NULL );
    buildContext.skeletons.push_back( NULL );
    return;
  }

  //skeleton mapping
  sasSkeletonMapping* skeletonMapping = sasNew sasSkeletonMapping();
  skeletonMapping->_boneMappings.resize( mesh->mNumBones );

  for( unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++ )
  {
    sasStringId currentBoneId = sasStringId::build( mesh->mBones[ boneIndex ]->mName.data );
    skeletonMapping->_boneMappings[ boneIndex ]._boneId = currentBoneId;
    skeletonMapping->_boneMappings[ boneIndex ]._boneIndex = boneIndex;//buildContext.skeletonHierarchy->findBoneIndex( currentBoneId );
    skeletonMapping->_boneMappings[ boneIndex ]._vertexInfluences.resize( mesh->mBones[ boneIndex ]->mNumWeights );
    
    for( unsigned int weightIndex = 0; weightIndex < mesh->mBones[ boneIndex ]->mNumWeights; weightIndex++ )
    {
      skeletonMapping->_boneMappings[ boneIndex ]._vertexInfluences[ weightIndex ]._vertexIndex = mesh->mBones[ boneIndex ]->mWeights[ weightIndex ].mVertexId;
      skeletonMapping->_boneMappings[ boneIndex ]._vertexInfluences[ weightIndex ]._boneWeight = mesh->mBones[ boneIndex ]->mWeights[ weightIndex ].mWeight;
    }
  }

  buildContext.skeletonMappings.push_back( skeletonMapping );

  //skeleton
  sasSkeleton* skeleton = sasNew sasSkeleton( mesh->mNumBones );
  for( unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++ )
  {
    skeleton->_boneMatrices[ boneIndex ].set( ( float* )( &mesh->mBones[ boneIndex ]->mOffsetMatrix ) );
    skeleton->_boneIdArray[ boneIndex ] = sasStringId::build( mesh->mBones[ boneIndex ]->mName.data );
  }
  skeleton->_skeletonId = sasStringId::build( "unnamed" );
  skeleton->_skeletonHierarchy = buildContext.skeletonHierarchy;

  buildContext.skeletons.push_back( skeleton );
}

sasSubMesh* extractMesh( const aiMesh* m, const sasSkeletonMapping* skeletonMapping, sasModelResourceData& buildContext )
{
  smVec4 boundingSphere;
  sasVertexBuffer* vb = extractMeshVertices( m, skeletonMapping, &boundingSphere );
  sasIndexBuffer* ib = extractMeshIndices( m );
  buildContext.vertexBuffers.push_back( vb );
  buildContext.indexBuffers.push_back( ib );

  sasSubMeshSetup setup;
  setup.vb = vb;
  setup.ib = ib;
  setup.firstIndex = 0;
  setup.nPrims = m->mNumFaces;
  setup.boundingSphere = boundingSphere;
  setup.material = buildContext.materials[ m->mMaterialIndex ];
  setup.isSkinned = ( vb->format().boneIndices == sasVertexFormat::BONEINDICES );

  sasSubMesh* subMesh = sasNew sasSubMesh( setup );
  return subMesh;
}

sasCamera* extractCamera( const aiCamera* c, sasModelResourceData& buildContext )
{
  sasCamera* camera = sasNew sasCamera();
  camera->setZPlanes( c->mClipPlaneNear, c->mClipPlaneFar );
  camera->setFOV( smRad2Deg( c->mHorizontalFOV ) );
  camera->setPosition( smVec4( c->mPosition.x, c->mPosition.y, c->mPosition.z, 1.f ) );
  return camera;
}

std::string findTexturePath( std::string& diffuseTexturePath, std::string& token )
{
  std::string key ("_diff.");
  size_t found;

  std::string res = diffuseTexturePath;

  found = res.rfind( key );
  if ( found != std::string::npos )
  {
    res.replace( found, key.length(), token );
  }
  else
  {
    // fallback to adding suffix
    size_t p = res.find_last_of( '.' );
    res.erase( p + 3 );
    res.erase( p + 2 );
    res.erase( p + 1 );
    res.erase( p );

    res += token + "tga";
  }

  return res;
}

sasMaterial* extractMaterial( const aiMaterial* m, sasModelResourceData& buildContext, const char* basepath )
{
  aiString path;
  aiTextureMapping mapping;
  size_t uvindex;
  std::vector< sasMaterialLayer > layers;
  size_t diffuseMapCnt = m->GetTextureCount( aiTextureType_DIFFUSE );
  for( size_t i = 0; i < diffuseMapCnt; i++ )
  {      
    aiReturn ret = m->GetTexture( aiTextureType_DIFFUSE, i, &path, &mapping, (unsigned int*)&uvindex );
    if( ret == aiReturn_SUCCESS && 
        mapping == aiTextureMapping_UV )
    {
      std::string texturepath = basepath;
      std::replace( texturepath.begin(), texturepath.end(), '\\', '/' );
      size_t p = texturepath.find_last_of( '/' );
      texturepath.erase( p+1 );
      texturepath += path.data;
      sasResource* texture = sasResourceMng::singleton().load( texturepath.c_str() );

      if( texture )
      {
        sasMaterialLayer layer;
        layer.texture = (sasTextureResource*)texture;
        layer.type = sasMaterialLayer::DiffuseMapLayer;
        layers.push_back( layer );

        //Normal map if it exists
        std::string token("_ddn.");
        std::string nmapTexturePath = findTexturePath( texturepath, token );
        sasResource* normalMap = sasResourceMng::singleton().load( nmapTexturePath.c_str() );

        if( normalMap )
        {
          sasMaterialLayer subLayer;
          subLayer.texture = (sasTextureResource*)normalMap;
          subLayer.type = sasMaterialLayer::NormalMapLayer;
          layers.push_back( subLayer );
        }

        //Spec map if it exists
        token = std::string("_spec.");
        std::string smapTexturePath = findTexturePath( texturepath, token );
        sasResource* specMap = sasResourceMng::singleton().load( smapTexturePath.c_str() );

        if( specMap )
        {
          sasMaterialLayer subLayer;
          subLayer.texture = (sasTextureResource*)specMap;
          subLayer.type = sasMaterialLayer::SpecMapLayer;
          layers.push_back( subLayer );
        }

        //Mask map if it exists
        token = std::string("_mask.");
        std::string mmapTexturePath = findTexturePath( texturepath, token );
        sasResource* maskMap = sasResourceMng::singleton().load( mmapTexturePath.c_str() );

        if( maskMap )
        {
          sasMaterialLayer subLayer;
          subLayer.texture = (sasTextureResource*)maskMap;
          subLayer.type = sasMaterialLayer::MaskMapLayer;
          layers.push_back( subLayer );
        }

        //Height map if it exists
        token = std::string("_height.");
        std::string hmapTexturePath = findTexturePath( texturepath, token );
        sasResource* heightMap = sasResourceMng::singleton().load( hmapTexturePath.c_str() );

        if( heightMap )
        {
          sasMaterialLayer subLayer;
          subLayer.texture = (sasTextureResource*)heightMap;
          subLayer.type = sasMaterialLayer::HeightMapLayer;
          layers.push_back( subLayer );
        }
      }
    }    
  }

  if( layers.empty() )
  {
    sasMaterialLayer layer;    
    layers.push_back( layer );
  }
  
  sasMaterial* material = sasNew sasMaterial( &layers[0], layers.size() );

  return material;
}

size_t copy( const aiVector3D* src, unsigned char* dst ) 
{
  float* dstf = (float*)dst;
  dstf[0] = src->x;
  dstf[1] = src->y;
  dstf[2] = src->z;
  return 12;
}

size_t copy2( const aiVector3D* src, unsigned char* dst ) 
{
  float* dstf = (float*)dst;
  dstf[0] = src->x;
  dstf[1] = src->y;  
  return 8;
}

size_t copy4f( const float* src, unsigned char* dst )
{
  float* dstf = ( float* )dst;
  dstf[ 0 ] = src[ 0 ];
  dstf[ 1 ] = src[ 1 ];
  dstf[ 2 ] = src[ 2 ];
  dstf[ 3 ] = src[ 3 ];
  return 16;
}

size_t copy4i( const int* src, unsigned char* dst )
{
  int* dsti = ( int* )dst;
  dsti[ 0 ] = src[ 0 ];
  dsti[ 1 ] = src[ 1 ];
  dsti[ 2 ] = src[ 2 ];
  dsti[ 3 ] = src[ 3 ];
  return 16;
}

sasVertexBuffer* extractMeshVertices( const aiMesh* mesh, const sasSkeletonMapping* skeletonMapping, smVec4* boundingSphere )
{
  sasVertexBuffer* result = 0;

  sasVertexFormat format;
  format.position = mesh->HasPositions() ? sasVertexFormat::POSITION_3F : 0;
  format.normal   = mesh->HasNormals() ? sasVertexFormat::NORMAL_3F : 0;
  format.tangent  = mesh->HasTangentsAndBitangents() ? sasVertexFormat::TANGENT_3F : 0;
  format.uv0      = mesh->HasTextureCoords(0) ? sasVertexFormat::UV_2F : 0;
  format.boneIndices = mesh->HasBones() ? sasVertexFormat::BONEINDICES : 0;
  format.boneWeights = mesh->HasBones() ? sasVertexFormat::BONEWEIGHTS : 0;
  
  result = sasNew sasVertexBuffer( format, mesh->mNumVertices );
  aiVector3D minPos(FLT_MAX, FLT_MAX, FLT_MAX);
  aiVector3D maxPos(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  aiVector3D* currentVertPos;
  int boneIndices[ 4 ];
  float boneWeights[ 4 ];
  unsigned char* ptr = (unsigned char*)result->map( false );
  if( ptr )
  {
    for( size_t i=0; i<result->verticesCount(); i++ )
    {
      if( mesh->HasPositions() )
      {
        ptr += copy( mesh->mVertices+i, ptr );
        currentVertPos = mesh->mVertices+i;
        getMinVector(minPos, *currentVertPos, minPos);
        getMaxVector(maxPos, *currentVertPos, maxPos);
      }      
      if( mesh->HasNormals() )
      {
        ptr += copy( mesh->mNormals+i, ptr );      
      }
      if( mesh->HasTangentsAndBitangents() )
      {
        ptr += copy( mesh->mTangents+i, ptr );      
      }
      if( mesh->HasTextureCoords(0) )
      {
        ptr += copy2( mesh->mTextureCoords[0]+i, ptr );      
      }
      if( mesh->HasBones() && skeletonMapping != NULL )
      {
        skeletonMapping->findBoneMappingForVertex( i, boneIndices[ 0 ], boneWeights[ 0 ], boneIndices[ 1 ], boneWeights[ 1 ], boneIndices[ 2 ], boneWeights[ 2 ], boneIndices[ 3 ], boneWeights[ 3 ] );
        ptr += copy4i( &boneIndices[ 0 ], ptr );
        ptr += copy4f( &boneWeights[ 0 ], ptr );
      }
    }
    result->unmap();
  }

  if(boundingSphere)
  {
    smVec3 minAABB = smVec3(minPos.x, minPos.y, minPos.z);
    smVec3 maxAABB = smVec3(maxPos.x, maxPos.y, maxPos.z);
    smVec3 centrePoint = smVec3(minAABB + maxAABB) / 2.f;
    float radius = smLength3(maxAABB - centrePoint);
    *boundingSphere = smVec4(centrePoint.X, centrePoint.Y, centrePoint.Z, radius);
  }

  return result;
}

sasIndexBuffer*  extractMeshIndices( const aiMesh* mesh )
{
  sasIndexBuffer* result = sasNew sasIndexBuffer( mesh->mNumFaces * 3 );

  unsigned short* ptr = result->map();
  if( ptr )
  {
    for( size_t i=0; i<result->indicesCount() / 3; i++ )
    {
      sasASSERT( mesh->mFaces[i].mNumIndices == 3 );
      ptr[0] = mesh->mFaces[i].mIndices[0];
      ptr[1] = mesh->mFaces[i].mIndices[1];
      ptr[2] = mesh->mFaces[i].mIndices[2];
      ptr += 3;
    }
    result->unmap();
  }

  return result;
}

void extractInstanceNodes( const aiScene* scene, const aiNode* node, sasSkeletonNode* parent, sasModelResourceData& buildContext )
{
  extractNodes( scene, node, parent, buildContext, false );
}

void extractSkeletonHierarchyNodes( const aiScene* scene, const aiNode* node, sasSkeletonNode* parent, sasModelResourceData& buildContext )
{
  extractNodes( scene, node, parent, buildContext, true );
}

void extractNodes( const aiScene* scene, const aiNode* node, sasSkeletonNode* parent, sasModelResourceData& buildContext, bool extractSkeletonHierarchy )
{
  //process current node
  sasSkeletonNode* currentProcessedNode = extractNode( scene, node, parent, buildContext, extractSkeletonHierarchy );

  for( size_t i=0; i<node->mNumChildren; i++ )
  {
    aiNode* n = node->mChildren[ i ];

    //recurse
    extractNodes( scene, n, currentProcessedNode, buildContext, extractSkeletonHierarchy );
  }
}

sasSkeletonNode* extractNode( const aiScene* scene, const aiNode* node, sasSkeletonNode* parent, sasModelResourceData& buildContext, bool extractSkeletonHierarchy )
{
  if( !extractSkeletonHierarchy )
  {
    //extract mesh instance
    if( node->mNumMeshes != 0 )
    {
      extractInstance( scene, node, buildContext );
    }
    return NULL;
  }

  //extract skeleton hierarchy
  if( buildContext.skeletonHierarchy != NULL )
  {
    buildContext.skeletonHierarchy->_numNodes++;
    sasSkeletonNode* newNode = &buildContext.skeletonHierarchy->_skeletonNodes[ buildContext.skeletonHierarchy->_numNodes - 1 ];
    newNode->_name = sasStringId::build( node->mName.data );
    newNode->_numChildren = node->mNumChildren;
    newNode->_transform.set( (float*)&node->mTransformation );
    newNode->_parent = parent;
    bool registeredWithParent = false;
    if( parent )
    {
      for( unsigned int i = 0; i < parent->_numChildren; i++ )
      {
        if( parent->_children[ i ] == NULL )
        {
          parent->_children[ i ] = newNode;
          registeredWithParent = true;
          break;
        }
      }
      sasASSERT( registeredWithParent );
    }
    else
    {
      buildContext.skeletonHierarchy->_rootNode = newNode;
    }
    return newNode;
  }

  return NULL;
}

void extractInstance( const aiScene* scene, const aiNode* node, sasModelResourceData& buildContext )
{
  typedef sasSubMesh* sasSubMeshPtr;
  sasSubMesh** sms = new sasSubMeshPtr[ node->mNumMeshes ];// sasSTACK_ALLOC( sasSubMesh*, node->mNumMeshes );  

  for( size_t i=0; i<node->mNumMeshes; i++ )
  {
    size_t mi = node->mMeshes[i];    
    sms[i] = buildContext.subMeshes[mi];
  }

  sasMesh* mesh = sasNew sasMesh( sms, node->mNumMeshes );
  buildContext.meshes.push_back( mesh );

  sasGeometryLOD lod;
  lod.mesh = mesh;
  lod.distance = -1.f;

  smAABB aabb = computeBoundingBox( &lod, 1 );

  mesh->setBoundingSphere( aabb.computeBoundingSphere() );

  sasGeometry* geometry = sasNew sasGeometry( &lod, 1 );
  buildContext.geometries.push_back( geometry );

  sasStagingInstance* instance = sasNew sasStagingInstance( geometry );

  smMat44 transform;
  transform.set( (float*)&node->mTransformation );  
  instance->setTransform( transform );
  buildContext.instances.push_back( instance );

  delete [] sms;
}

smAABB computeBoundingBox( sasGeometryLOD* lods, size_t nLods )
{
  smAABB* aabbs = sasSTACK_ALLOC( smAABB, nLods );  

  for( size_t i=0; i<nLods; i++ )
  {
    aabbs[ i ] = computeBoundingBox( lods[i].mesh );
  }

  smAABB result = aabbs[0];
  for( size_t i=1; i<nLods; i++ )
  {
    result.unionWith( aabbs[i] );
  }

  return result;
}

smAABB computeBoundingBox( sasMesh* mesh )
{
  smAABB result;
  for( size_t i=0; i<mesh->subMeshCount(); i++ )
  {
    smAABB aabb = computeBoundingBox( mesh->subMesh( i ) );
    result.unionWith( aabb );
  }
  return result;
}

smAABB computeBoundingBox( sasSubMesh* mesh )
{
  smAABB result;
  sasVertexBuffer* vb = mesh->vertexBuffer();
  unsigned char* data = (unsigned char*)vb->map( true );
  if( data )
  {
    sasVertexReader r( vb->format(), data );
    float position[3];
    float xMin = FLT_MAX;
    float xMax = -FLT_MAX;
    float yMin = FLT_MAX;
    float yMax = -FLT_MAX;
    float zMin = FLT_MAX;
    float zMax = -FLT_MAX;
    for( size_t i=0; i<vb->verticesCount(); i++ )
    {
      r.readPosition( position );
      xMin = smMin( xMin, position[0] );
      xMax = smMax( xMax, position[0] );
      yMin = smMin( yMin, position[1] );
      yMax = smMax( yMax, position[1] );
      zMin = smMin( zMin, position[2] );
      zMax = smMax( zMax, position[2] );
      r.next();
    }
    result = smAABB( smVec4( xMin, yMin, zMin, 1.f ), smVec4( xMax, yMax, zMax, 1.f ) );
    vb->unmap();
  }
  return result;
}

void extractAnimations( const char* modelName, const aiScene* scene, sasModelResourceData& buildContext )
{
  for( unsigned int i = 0; i < scene->mNumAnimations; i++ )
  {
    extractAnimation( modelName, scene->mAnimations[ i ], buildContext );
  }
}

void extractAnimation( const char* modelName, const aiAnimation* animation, sasModelResourceData& buildContext )
{
  sasASSERT( animation != NULL );
  std::string animName;
  if( animation->mName.length == 0 )
  {
    animName = modelName;
    animName += std::to_string( buildContext.compatibleAnimations.size() );
  }
  else
  {
    animName = animation->mName.data;
  }
  sasStringId animId = sasStringId::build( animName.c_str() );
  sasAnimation* anim = sasNew sasAnimation( animId, animation->mDuration, animation->mTicksPerSecond, animation->mNumChannels );

  for( unsigned int channelIndex = 0; channelIndex < animation->mNumChannels; channelIndex++ )
  {
    sasAnimationChannel* channel =  &( anim->_channels[ channelIndex ] );
    aiNodeAnim* assimpChannel = animation->mChannels[ channelIndex ];

    channel->_boneId = sasStringId::build( assimpChannel->mNodeName.data );
    channel->_numPositionFrames = assimpChannel->mNumPositionKeys;
    channel->_numRotationFrames = assimpChannel->mNumRotationKeys;
    channel->_numScalingFrames = assimpChannel->mNumScalingKeys;
    
    for( unsigned int frameIndex = 0; frameIndex < channel->_numPositionFrames; frameIndex++ )
    {
      channel->_positionFrames[ frameIndex ]._position.X = assimpChannel->mPositionKeys[ frameIndex ].mValue.x;
      channel->_positionFrames[ frameIndex ]._position.Y = assimpChannel->mPositionKeys[ frameIndex ].mValue.y; 
      channel->_positionFrames[ frameIndex ]._position.Z = assimpChannel->mPositionKeys[ frameIndex ].mValue.z;
      channel->_positionFrames[ frameIndex ]._time = assimpChannel->mPositionKeys[ frameIndex ].mTime;
    }

    for( unsigned int frameIndex = 0; frameIndex < channel->_numScalingFrames; frameIndex++ )
    {
      channel->_scalingFrames[ frameIndex ]._scale = smVec3( assimpChannel->mScalingKeys[ frameIndex ].mValue.x, 
                                                              assimpChannel->mScalingKeys[ frameIndex ].mValue.y, 
                                                              assimpChannel->mScalingKeys[ frameIndex ].mValue.z );
      channel->_scalingFrames[ frameIndex ]._time = assimpChannel->mScalingKeys[ frameIndex ].mTime;
    }

    for( unsigned int frameIndex = 0; frameIndex < channel->_numRotationFrames; frameIndex++ )
    {
      channel->_rotationFrames[ frameIndex ]._rotation = assimpChannel->mRotationKeys[ frameIndex ].mValue;
      channel->_rotationFrames[ frameIndex ]._time = assimpChannel->mRotationKeys[ frameIndex ].mTime;
    }
  }

  buildContext.compatibleAnimations.push_back( anim );
  sasResourceMng::singleton().registerAnimation( anim );
}


/*
  physics data
*/

bool extractPhysicsBoxList( sasModelResourceData& mrd, offsetBox** boxList, uint32_t& numBoxes )
{
  static const uint32_t kMaxBoxes = 1024;
  offsetBox* boxes = static_cast< offsetBox* >( sasMalloc( kMaxBoxes * sizeof( offsetBox ) ) );
  numBoxes = 0;
  for( size_t instIdx = 0; instIdx < mrd.instances.size(); instIdx++ )
  {
    sasStagingInstance* inst = mrd.instances[ instIdx ];
    smMat44 instXform = inst->transform();
    sasGeometry* geo = inst->geometry();
    for( size_t lodsIdx = 0; lodsIdx < geo->numLods(); lodsIdx++ )
    {
      sasMesh* mesh = geo->lod( lodsIdx );
      for( size_t submeshIdx = 0; submeshIdx < mesh->subMeshCount(); submeshIdx++ )
      {
        sasSubMesh* subMesh = mesh->subMesh( submeshIdx );
        boxes[ numBoxes ].aabb = computeBoundingBox( subMesh );
        boxes[ numBoxes ].xform = instXform;
        sasASSERT( numBoxes < ( kMaxBoxes - 1 ) );
        ++numBoxes;
      }
    }
  }

  *boxList = ( offsetBox* )sasMalloc( sizeof( offsetBox ) * numBoxes );
  memcpy( *boxList, &boxes[ 0 ], numBoxes * sizeof( offsetBox ) );

  sasFree( boxes );

  return true;
}

bool serializePhysicsData( sasVirtualFile& vFile, offsetBox* boxList, uint32_t numBoxes )
{
  vFile.write( numBoxes );
  for( uint32_t i = 0; i < numBoxes; i++ )
  {
    for( uint32_t j = 0; j < 3; j++ )
      vFile.write( boxList[ i ].aabb._min[ j ] );
    for( uint32_t j = 0; j < 3; j++ )
      vFile.write( boxList[ i ].aabb._max[ j ] );

    for( uint32_t j = 0; j < 4; j++ )
      vFile.write( boxList[ i ].xform.L0[ j ] );
    for( uint32_t j = 0; j < 4; j++ )
      vFile.write( boxList[ i ].xform.L1[ j ] );
    for( uint32_t j = 0; j < 4; j++ )
      vFile.write( boxList[ i ].xform.L2[ j ] );
    for( uint32_t j = 0; j < 4; j++ )
      vFile.write( boxList[ i ].xform.L3[ j ] );
  }
  return true;
}

/*
  Model resource serialization to internal engine format
*/

bool serializeVertexBuffer( sasVirtualFile& vFile, const sasVertexBuffer* vb )
{
  static const uint32_t kVbVersion = 1;
  vFile.write( kVbVersion );
  vFile.write( vb->format().flags );
  vFile.write( vb->verticesCount() );
  vFile.write( vb->size() );
  vFile.write( reinterpret_cast< uint8_t* >( vb->buffer() ), vb->size() );

  return true;
}

bool serializeIndexBuffer( sasVirtualFile& vFile, const sasIndexBuffer* ib )
{
  static const uint32_t kIbVersion = 1;
  vFile.write( kIbVersion );
  vFile.write( ib->indicesCount() );
  vFile.write( ib->size() );
  vFile.write( reinterpret_cast< uint8_t* >( ib->buffer() ), ib->size() );

  return true;
}

uint32_t findIndex( const sasModelResourceData& mrd, const sasVertexBuffer* vb )
{
  for( size_t i = 0; i < mrd.vertexBuffers.size(); i++ )
  {
    if( mrd.vertexBuffers[ i ] == vb )
      return i;
  }
  sasASSERT( false );
  return 0xffffffff;
}

uint32_t findIndex( const sasModelResourceData& mrd, const sasIndexBuffer* ib )
{
  for( size_t i = 0; i < mrd.indexBuffers.size(); i++ )
  {
    if( mrd.indexBuffers[ i ] == ib )
      return i;
  }
  sasASSERT( false );
  return 0xffffffff;
}

uint32_t findIndex( const sasModelResourceData& mrd, const sasMaterial* m )
{
  for( size_t i = 0; i < mrd.materials.size(); i++ )
  {
    if( mrd.materials[ i ] == m )
      return i;
  }
  sasASSERT( false );
  return 0xffffffff;
}

uint32_t findIndex( const sasModelResourceData& mrd, const sasSubMesh* sm )
{
  for( size_t i = 0; i < mrd.subMeshes.size(); i++ )
  {
    if( mrd.subMeshes[ i ] == sm )
      return i;
  }
  sasASSERT( false );
  return 0xffffffff;
}

uint32_t findIndex( const sasModelResourceData& mrd, const sasMesh* m )
{
  for( size_t i = 0; i < mrd.meshes.size(); i++ )
  {
    if( mrd.meshes[ i ] == m )
      return i;
  }
  sasASSERT( false );
  return 0xffffffff;
}

uint32_t findIndex( const sasModelResourceData& mrd, const sasGeometry* g )
{
  for( size_t i = 0; i < mrd.geometries.size(); i++ )
  {
    if( mrd.geometries[ i ] == g )
      return i;
  }
  sasASSERT( false );
  return 0xffffffff;
}

bool serializeSubmesh( sasVirtualFile& vFile, const sasSubMesh* sm, const sasModelResourceData& mrd )
{
  static const uint32_t kSubmeshVersion = 1;
  vFile.write( kSubmeshVersion );
  vFile.write( findIndex( mrd, sm->vertexBuffer() ) );
  vFile.write( findIndex( mrd, sm->indexBuffer() ) );
  vFile.write( sm->numPrims() );
  vFile.write( sm->firstIndex() );
  vFile.write( findIndex( mrd, sm->material() ) );
  vFile.write( sm->isSkinned() ? TRUE : FALSE );
  vFile.write( sm->boundingSphere().X );
  vFile.write( sm->boundingSphere().Y );
  vFile.write( sm->boundingSphere().Z );
  vFile.write( sm->boundingSphere().W );

  return true;
}


bool serializeMesh( sasVirtualFile& vFile, const sasMesh* m, const sasModelResourceData& mrd )
{
  static const uint32_t kMeshVersion = 1;
  vFile.write( kMeshVersion );
  vFile.write( m->subMeshCount() );
  for( uint32_t i = 0; i < m->subMeshCount(); i++ )
  {
    vFile.write( findIndex( mrd, m->subMesh( i ) ) );
  }
  vFile.write( m->boundingSphere()->X );
  vFile.write( m->boundingSphere()->Y );
  vFile.write( m->boundingSphere()->Z );
  vFile.write( m->boundingSphere()->W );

  return true;
}

bool serializeGeo( sasVirtualFile& vFile, const sasGeometry* g, const sasModelResourceData& mrd )
{
  static const uint32_t kGeoVersion = 1;
  vFile.write( kGeoVersion );
  vFile.write( g->numLods() );

  for( uint32_t i = 0; i < g->numLods(); i++ )
  {
    vFile.write( g->lodDistance( i ) );
    vFile.write( findIndex( mrd, g->lod( i ) ) );
  }

  return true;
}

bool serializeInstances( sasVirtualFile& vFile, const sasStagingInstance* inst, const sasModelResourceData& mrd )
{
  static const uint32_t kInstanceVersion = 1;
  vFile.write( kInstanceVersion );

  vFile.write( findIndex( mrd, inst->geometry() ) );

  for( uint32_t i = 0; i < 4; i++ )
    vFile.write( inst->transform().L0[ i ] );
  for( uint32_t i = 0; i < 4; i++ )
    vFile.write( inst->transform().L1[ i ] );
  for( uint32_t i = 0; i < 4; i++ )
    vFile.write( inst->transform().L2[ i ] );
  for( uint32_t i = 0; i < 4; i++ )
    vFile.write( inst->transform().L3[ i ] );

  return true;
}

bool serializeModelResource( sasVirtualFile& vFile, const sasModelResource* mr )
{
  static const uint32_t kModelResourceVersion = 5;
  static const uint32_t kTrue = 1;
  static const uint32_t kFalse = 0;
  vFile.write( kModelResourceVersion );

  const sasModelResourceData* mrd = &( mr->resourceData() );

  //materials
  vFile.write( mrd->materials.size() );
  for( size_t i = 0; i < mrd->materials.size(); i++ )
  {
    mrd->materials[ i ]->serialize( vFile );
  }

  //vbs
  vFile.write( mrd->vertexBuffers.size() );
  for( size_t i = 0; i < mrd->vertexBuffers.size(); i++ )
  {
    serializeVertexBuffer( vFile, mrd->vertexBuffers[ i ] );
  }

  //ibs
  vFile.write( mrd->indexBuffers.size() );
  for( size_t i = 0; i < mrd->indexBuffers.size(); i++ )
  {
    serializeIndexBuffer( vFile, mrd->indexBuffers[ i ] );
  }

  //submeshes
  vFile.write( mrd->subMeshes.size() );
  for( size_t i = 0; i < mrd->subMeshes.size(); i++ )
  {
    serializeSubmesh( vFile, mrd->subMeshes[ i ], *mrd );
  }

  //meshes
  vFile.write( mrd->meshes.size() );
  for( size_t i = 0; i < mrd->meshes.size(); i++ )
  {
    serializeMesh( vFile, mrd->meshes[ i ], *mrd );
  }
  
  //geometries
  vFile.write( mrd->geometries.size() );
  for( size_t i = 0; i < mrd->geometries.size(); i++ )
  {
    serializeGeo( vFile, mrd->geometries[ i ], *mrd );
  }

  //skeleton hierarchy
  if( mrd->skeletonHierarchy )
  {
    vFile.write( 1 );
    mrd->skeletonHierarchy->serialize( vFile );
  }
  else
  {
    vFile.write( 0 );
  }

  //skeletons
  vFile.write( mrd->skeletons.size() );
  for( size_t i = 0; i < mrd->skeletons.size(); i++ )
  {
    if( mrd->skeletons[ i ] != nullptr )
    {
      vFile.write( kTrue );
      mrd->skeletons[ i ]->serialize( vFile );
    }
    else
    {
      vFile.write( kFalse );
    }
  }

  //animations
  vFile.write( mrd->compatibleAnimations.size() );
  for( size_t i = 0; i < mrd->compatibleAnimations.size(); i++ )
  {
    mrd->compatibleAnimations[ i ]->serialize( vFile );
  }

  //instances
  vFile.write( mrd->instances.size() );
  for( size_t i = 0; i < mrd->instances.size(); i++ )
  {
    serializeInstances( vFile, mrd->instances[ i ], *mrd );
  }

  return true;
}


