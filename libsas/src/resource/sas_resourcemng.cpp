#include "sas_pch.h"
#include "sas_resourcemng.h"
#include "sas_resourceloader.h"
#include "sas_modelinstance.h"
#include "utils/sas_path.h"
#include "sas_resource.h"
#include "sas_model_resource.h"
#include "sas_resource.h"
#include "sas_splashscreen.h"
#include "render/sas_instance.h"
#include "animation/sas_animation.h"
#include "sas_camerapath.h"
#include "utils/sas_file.h"
#include "sas_texture_resource.h"
#include "resource/sas_model_loader.h"

sasResourceMng::sasResourceMng( const char* path )
  : _resourcePath( path )
{  
  _models.reserve( 1024 );
  _modelInstances.reserve( 2048 );
  _skinnedModelInstances.reserve( 512 );
  _staticModelInstances.reserve( 1536 );
  _cameraPaths.reserve( 32 );
}

sasResourceMng::~sasResourceMng()
{
  for( UniqueLoaders::const_iterator i=_uniqueLoaders.begin(); i!=_uniqueLoaders.end(); ++i ) 
  {
    sasDelete *i;
  }
}

bool sasResourceMng::registerLoader( const char* ext, sasResourceLoader* loader )
{
  sasASSERT( ext && loader );

  Loaders::const_iterator i = _loaders.find( ext );
  if( i!=_loaders.end() )
    return false;

  _loaders[ ext ] = loader;
  _uniqueLoaders.insert( loader );

  return true;
}

sasResource* sasResourceMng::load( const char* path )
{
  sasASSERT( path );

  const char* ext = sasPathGetExt( path );
  if( !ext )
  {
    sasDebugOutput("sasResourceMng::load - path: %s does not have any extension\n", path );
    sasASSERT( false );
    return 0;
  }

  Loaders::const_iterator i = _loaders.find( std::string( ext ) );
  if( i==_loaders.end() )
  {
    sasDebugOutput("sasResourceMng::load - no appropriate loaders found for file: %s. Unsupported extension: %s\n", path, ext );
    sasASSERT( false );
    return 0;
  }

  char filename[ 16 ];
  sasPathGetFilename( path, filename, 16 );

  sasResource* resource = (*i).second->load( path, filename );
  
  return resource;  
}

sasStringId sasResourceMng::createModelInstance( const char* path, const char* instanceName )
{
  std::string fullPath = _resourcePath;
  fullPath += path;

  char filename[ 16 ];
  sasPathGetFilename( path, filename, 16 );

  sasStringId modelName = sasStringId::build( filename );
  sasModelResource* modelResource = NULL;
  for( ModelArray::iterator it = _models.begin(); it != _models.end(); it++ )
  {
    if( (*it)->name() == modelName )
    {
      modelResource = *it;
      modelResource->incRefCount();
      break;
    }
  }

  if( modelResource == NULL )
  {
    //try to load sasModel
    char newPath[ MAX_PATH ];
    sasPathRemoveExtension( fullPath.c_str(), newPath, MAX_PATH );
    std::string sasFmtModelPath = newPath;
    sasFmtModelPath += ".sasModel";
    modelResource = reinterpret_cast< sasModelResource* >( load( sasFmtModelPath.c_str() ) );

    //failed, so load assimp version which will generate sasModel for next time
    if( modelResource == nullptr )
    {
      modelResource = reinterpret_cast< sasModelResource* >( load( fullPath.c_str() ) );
    }
    
    if( modelResource == NULL )
    {
      return sasStringId::build( "invalid" );
    }
    _models.push_back( modelResource );
  }

  sasStringId instanceId;
  if( instanceName )
  {
    instanceId = sasStringId::build( instanceName );
  }
  else
  {
    std::string auto_generated_name = "unnamed_inst_";
    static unsigned long long generatedInstanceCount = 0;
    auto_generated_name += std::to_string( generatedInstanceCount++ );
    instanceId = sasStringId::build( auto_generated_name.c_str() );
  }

  sasModelInstance* modelInst = sasNew sasModelInstance( modelResource, instanceId );
  _modelInstances.push_back( modelInst );

  //push into utility lists
  if( modelInst->skinned() )
  {
    _skinnedModelInstances.push_back( modelInst );
  }
  else
  {
    _staticModelInstances.push_back( modelInst );
  }

  return instanceId;
}

sasStringId sasResourceMng::createBlobModelInstance( sasStringId instanceId )
{
  static const sasStringId kBlobRscId = sasStringId::build( "sasBlobModel" );

  sasModelResource* modelResource = NULL;
  for( ModelArray::iterator it = _models.begin(); it != _models.end(); it++ )
  {
    if( (*it)->name() == kBlobRscId )
    {
      modelResource = *it;
      modelResource->incRefCount();
      break;
    }
  }

  if( modelResource == nullptr )
  {
    modelResource = reinterpret_cast< sasModelResource* >( createBlobModelResource( kBlobRscId ) );

    if( modelResource == NULL )
    {
      sasASSERT( false );
      return sasStringId::build( "invalid" );
    }
    _models.push_back( modelResource );
  }

  sasModelInstance* modelInst = sasNew sasModelInstance( modelResource, instanceId );
  _modelInstances.push_back( modelInst );

  if( modelInst->skinned() )
  {
    _skinnedModelInstances.push_back( modelInst );
  }
  else
  {
    _staticModelInstances.push_back( modelInst );
  }

  return instanceId;
}

void sasResourceMng::deleteModelInstance( sasStringId id )
{
  for( size_t i = 0; i < _modelInstances.size(); i++ )
  {
    if( _modelInstances[ i ]->instanceId() == id )
    {
      //found it

      if( _modelInstances[ i ]->skinned() )
      {
        for( size_t j = 0; j < _skinnedModelInstances.size(); j++ )
        {
          if( _skinnedModelInstances[ j ]->instanceId() == id )
          {
            _skinnedModelInstances[ j ] = _skinnedModelInstances[ _skinnedModelInstances.size() - 1 ];
            _skinnedModelInstances.pop_back();
            break;
          }
        }
      }
      else
      {
        for( size_t j = 0; j < _staticModelInstances.size(); j++ )
        {
          if( _staticModelInstances[ j ]->instanceId() == id )
          {
            _staticModelInstances[ j ] = _staticModelInstances[ _staticModelInstances.size() - 1 ];
            _staticModelInstances.pop_back();
            break;
          }
        }
      }

      sasDelete _modelInstances[ i ];
      _modelInstances[ i ] = _modelInstances[ _modelInstances.size() - 1 ];
      _modelInstances.pop_back();
      break;
    }
  }
}

sasModelInstance* sasResourceMng::findModelInstance( sasStringId instance_id )
{
  for( ModelInstanceArray::iterator it = _modelInstances.begin(); it != _modelInstances.end(); it++ )
  {
    if( (*it)->instanceId() == instance_id )
    {
      return *it;
    }
  }
  sasDebugOutput( "sasResourceMng::findModelInstance - could not find model instance: %s\n", instance_id );

  return NULL;
}

void sasResourceMng::unloadAll()
{
  unloadCameraPaths();

  for( unsigned int i = 0; i < _splashScreens.size(); i++ )
  {
    sasDelete _splashScreens[ i ];
  }
  _splashScreens.clear();

  for( unsigned int i = 0; i < _animations.size(); i++ )
  {
    sasDelete _animations[ i ];
  }
  _animations.clear();

  for( unsigned int i = 0; i < _modelInstances.size(); i++ )
  {
    sasDelete _modelInstances[ i ];
  }
  _modelInstances.clear();

  for( unsigned int i = 0; i < _models.size(); i++ )
  {
    unload( _models[ i ] );
  }
  _models.clear();
}

void sasResourceMng::unload( sasResource* resource )
{
    sasASSERT( resource );    
    sasDelete resource;
}

void sasResourceMng::unregisterModelRsc( sasModelResource* rsc )
{
  for( unsigned int i = 0; i < _models.size(); i++ )
  {
    if( rsc == _models[ i ] )
    {
      _models[ i ] = _models[ _models.size() - 1 ];
      _models.pop_back();
      return;
    }
  }

  sasDebugOutput( "sasResourceMng::unregisterModelRsc - could not unregister model, was not found in rscMng model list.\n" );
}

void sasResourceMng::unloadCameraPaths()
{
  for( unsigned int i = 0; i < _cameraPaths.size(); i++ )
  {
    sasDelete _cameraPaths[ i ];
  }
  _cameraPaths.clear();
}

void sasResourceMng::registerAnimation( sasAnimation* animation )
{
  _animations.push_back( animation );
}

sasAnimation* sasResourceMng::getAnimation( sasStringId animationId )
{
  sasAnimation* anim = NULL;
  bool found = false;
  for( AnimationArray::iterator it = _animations.begin(); it != _animations.end(); it++ )
  {
    if( ( *it )->_animationId == animationId )
    {
      anim = *it;
      found = true;
      break;
    }
  }

  if( !found )
    sasDebugOutput( "sasResourceMng::getAnimation - failed to find animation %d\n", animationId );

  return anim;
}

void sasResourceMng::stepPreRender()
{
  for( size_t i = 0; i < _skinnedModelInstances.size(); i++ )
  {
    _skinnedModelInstances[ i ]->stepPreRender();
  }
}

void sasResourceMng::createSplashScreen( sasStringId id, const char* path, const smVec2& pos, const smVec2& scale )
{
  if( findSplashScreen( id ) != NULL )
    return;

  sasSplashScreen* sc = sasNew sasSplashScreen( id, path, scale, pos );
  _splashScreens.push_back( sc );
}

sasSplashScreen* sasResourceMng::findSplashScreen( sasStringId id )
{
  for( SplashScreenArray::iterator it = _splashScreens.begin(); it != _splashScreens.end(); it++ )
  {
    if( (*it)->id() == id )
    {
      return *it;
    }
  }

  return NULL;
}

void sasResourceMng::createCameraPath( sasStringId id, const char* name )
{
  if( findCameraPath( id ) != NULL )
    return;

  sasCameraPath* cp = sasNew sasCameraPath( id, name );
  _cameraPaths.push_back( cp );
}

void sasResourceMng::deleteCameraPath( sasStringId id )
{
  bool found = false;
  size_t index = 0;
  for( size_t i = 0; i < _cameraPaths.size(); i++ )
  {
    if( _cameraPaths[ i ]->id() == id )
    {
      found = true;
      index = i;
      break;
    }
  }

  if( !found )
    return;

  sasDelete _cameraPaths[ index ];
  _cameraPaths[ index ] = _cameraPaths[ _cameraPaths.size() - 1 ];
  _cameraPaths.pop_back();
}

sasCameraPath* sasResourceMng::findCameraPath( sasStringId id )
{
  for( CameraPathArray::iterator it = _cameraPaths.begin(); it != _cameraPaths.end(); it++ )
  {
    if( (*it)->id() == id )
    {
      return *it;
    }
  }

  return NULL;
}

sasCameraPath* sasResourceMng::findCameraPath( unsigned int index )
{
  size_t idx = static_cast< size_t >( index );
  if( idx >= _cameraPaths.size() )
    return NULL;

  return _cameraPaths[ idx ];
}



/*
  sasTextureMng
*/
sasTextureMng::sasTextureMng()
  : _pendingTexturesPullIndex( 0 )
  , _pendingTextureAddIndex( 0 )
{
  _textures.reserve( 2048 );
}

sasTextureMng::~sasTextureMng()
{
  sasASSERT( _textures.size() == 0 );
  sasASSERT( _pendingTextureAddIndex == _pendingTexturesPullIndex );
}

sasTextureResource* sasTextureMng::getTexture( sasTextureId texId ) const
{
  size_t numTextures = _textures.size();
  for( size_t i = 0; i < numTextures; i++ )
  {
    if( _textures[ i ]->id() == texId )
    {
      return _textures[ i ];
    }
  }

  uint32_t texIdLow = static_cast< uint32_t >( texId & ( uint64_t )0xffffffff );
  uint32_t texIdHigh = static_cast< uint32_t >( texId >> 32 );
  sasDebugOutput( "sasTextureMng::getTexture - failed to find texture requested ( id: %d %d )\n", texIdHigh, texIdLow );
  return NULL;
}

void sasTextureMng::registerTexture( sasTextureResource* texRsc )
{
  ++_pendingTextureAddIndex;
  _pendingTextures[ _pendingTextureAddIndex - 1 ] = texRsc;
}

void sasTextureMng::unregisterTexture( sasTextureResource* texRsc )
{
  sasTextureId texId = texRsc->id();
  size_t numTextures = _textures.size();
  for( size_t i = 0; i < numTextures; i++ )
  {
    if( _textures[ i ]->id() == texId )
    {
      ++_pendingDeleteAddIndex;
      _pendingIndicesToDelete[ _pendingDeleteAddIndex - 1 ] = static_cast< uint32_t >( i );
      return;
    }
  }

  uint32_t texIdLow = static_cast< uint32_t >( texId & ( uint64_t )0xffffffff );
  uint32_t texIdHigh = static_cast< uint32_t >( texId >> 32 );
  sasDebugOutput( "sasTextureMng::unregisterTexture - failed to find texture requested for deletion ( id: %d %d )\n", texIdHigh, texIdLow );
}

void sasTextureMng::stepPostFrame()
{
  while( _pendingTexturesPullIndex != _pendingTextureAddIndex )
  {
    ++_pendingTexturesPullIndex;
    _textures.push_back( _pendingTextures[ _pendingTexturesPullIndex - 1 ] );
  }

  while( _pendingDeletePullIndex != _pendingDeleteAddIndex )
  {
    ++_pendingDeletePullIndex;
    uint32_t deleteIndex = _pendingIndicesToDelete[ _pendingDeletePullIndex - 1 ];
    //_textures[ deleteIndex ]
  }
}
