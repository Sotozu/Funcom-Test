#pragma once 

#include "utils/sas_singleton.h"
#include <map>
#include <string>
#include "utils/sas_stringid.h"

class sasResource;
class sasResourceLoader;
class sasModelResource;
class sasInstance;
class sasModelInstance;
class sasAnimation;
class sasSplashScreen;
class sasCameraPath;
class sasTextureResource;

class sasResourceMng : public sasSingleton< sasResourceMng >
{
  sasNO_COPY( sasResourceMng );
  sasMEM_OP( sasResourceMng );

public:
  sasResourceMng( const char* path );
  ~sasResourceMng();

public:
  bool registerLoader( const char* ext, sasResourceLoader* loader );

  sasResource* load( const char* path );

  sasStringId createModelInstance( const char* path, const char* instanceName = NULL );
  sasStringId createBlobModelInstance( sasStringId instanceName );
  void        deleteModelInstance( sasStringId id );
  void        createSplashScreen( sasStringId id, const char* path, const smVec2& pos, const smVec2& scale );
  void        createCameraPath( sasStringId id, const char* name );
  void        deleteCameraPath( sasStringId id );
  size_t      numCameraPaths() const { return _cameraPaths.size(); }

  sasModelInstance* findModelInstance( sasStringId instance_id );
  sasSplashScreen*  findSplashScreen( sasStringId id );
  sasCameraPath*    findCameraPath( sasStringId id );
  sasCameraPath*    findCameraPath( unsigned int index );

  void unload( sasResource* resource );
  void unloadAll();
  void unloadCameraPaths();
  void unregisterModelRsc( sasModelResource* rsc );

  void registerAnimation( sasAnimation* animation );
  sasAnimation* getAnimation( sasStringId animationId );
  
  void stepPreRender();

  const char* resourcePath() const { return _resourcePath.c_str(); }

private:
  typedef std::map< std::string, sasResourceLoader* > Loaders;
  typedef std::set< sasResourceLoader* >              UniqueLoaders;
  typedef std::vector< sasModelResource* >            ModelArray;
  typedef std::vector< sasModelInstance* >            ModelInstanceArray;
  typedef std::vector< sasAnimation* >                AnimationArray;
  typedef std::vector< sasSplashScreen* >             SplashScreenArray;
  typedef std::vector< sasCameraPath* >               CameraPathArray;

  Loaders       _loaders;  
  UniqueLoaders _uniqueLoaders;
  std::string   _resourcePath;
  ModelArray    _models;
  ModelInstanceArray _modelInstances;

  ModelInstanceArray _skinnedModelInstances;
  ModelInstanceArray _staticModelInstances;

  AnimationArray    _animations;
  SplashScreenArray _splashScreens;
  CameraPathArray   _cameraPaths;

};


/*
  sasTextureMng
*/

class sasTextureMng : public sasSingleton< sasTextureMng >
{
  sasNO_COPY( sasTextureMng );
  sasMEM_OP( sasTextureMng );

public:
  sasTextureMng();
  ~sasTextureMng();

  sasTextureResource* getTexture( sasTextureId texId ) const;
  void                registerTexture( sasTextureResource* texRsc );
  void                unregisterTexture( sasTextureResource* texRsc );

  void                stepPostFrame();

private:
  typedef std::vector< sasTextureResource* > TextureResArray;
  TextureResArray _textures;
  
  //pending textures ring buffer, safe to add to from any thread
  static const unsigned int kMaxNumPendingTextures = 512;
  sasTextureResource* _pendingTextures[ kMaxNumPendingTextures ];
  volatile long       _pendingTextureAddIndex;
  volatile long       _pendingTexturesPullIndex;

  uint32_t _pendingIndicesToDelete[ kMaxNumPendingTextures ];
  volatile long       _pendingDeleteAddIndex;
  volatile long       _pendingDeletePullIndex;


};