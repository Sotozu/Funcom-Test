#pragma once 

#include <string>
#include "utils/sas_stringid.h"

class sasTextureResource;
class sasRenderTarget;
class sasConstantBuffer;

class sasSplashScreen
{
  sasNO_COPY( sasSplashScreen );
  sasMEM_OP( sasSplashScreen );

public:
  sasSplashScreen( sasStringId splashScreenId, const char* texturePath, const smVec2& scale, const smVec2& position );
  ~sasSplashScreen();

  void setPosition( const smVec2& position ) { _position = position; }
  void setScale( const smVec2& scale ) { _scale = scale; }

  sasStringId id() const { return _id; }
  const smVec2& position() const { return _position; }
  const smVec2& scale() const { return _scale; }
  const sasTextureResource* texture() const { return _texture; }

  void render( sasRenderTarget* rt );

  static void setActiveSplashScreen( sasSplashScreen* splashScreen ) { sActiveSplashScreen = splashScreen; }
  static sasSplashScreen* getActiveSplashScreen() { return sActiveSplashScreen; }

  static void activateWidget( sasSplashScreen* widget );
  static void deactivateWidget( sasSplashScreen* widget );
  static void renderWidgets( sasRenderTarget* targetBuffer );

private:
  static sasSplashScreen* sActiveSplashScreen;
  static std::vector< sasSplashScreen* > sActiveWidgets;

  sasStringId _id;
  sasTextureResource* _texture;
  smVec2 _position;
  smVec2 _scale;

  sasConstantBuffer* _constants;
};
