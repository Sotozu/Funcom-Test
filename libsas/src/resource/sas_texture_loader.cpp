#include "sas_pch.h"
#include "sas_texture_loader.h"
#include "sas_texture_resource.h"
#include "sas_resourcemng.h"
#include "render/sas_texture.h"
#include "utils/sas_file.h"
#include "utils/sas_path.h"
#include "utils/sas_hash.h"

#include <il/il.h>
#include <nvtt.h>

using namespace nvtt;

void sasTextureRegisterLoader()
{
  sasTextureLoader* loader = sasNew sasTextureLoader;
  sasResourceMng::singleton().registerLoader( "tga", loader );  
  sasResourceMng::singleton().registerLoader( "sasTexArray", loader );
}

typedef std::vector< sasTextureMipData > MipDatas;

struct sasOutputHandler : public OutputHandler
{
  sasOutputHandler( MipDatas& mips )
    : _mips( mips ),
    _data( 0 )
  {
  }

  /// Indicate the start of a new compressed image that's part of the final texture.
  virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel)
  {
    sasTextureMipData mip;    
    
    mip.width = width;
    mip.height = height;
    mip.data = (unsigned char*)sasMalloc( size );
    mip.dataSize = size;    
    _mips.push_back( mip );
    _data = mip.data;
  }

  /// Output data. Compressed data is output as soon as it's generated to minimize memory allocations.
  virtual bool writeData(const void * data, int size)
  {    
    memcpy( _data, data, size );
    _data += size;
    return true;
  }

  unsigned char* _data;
  MipDatas&     _mips;
};

void FreeMip( sasTextureMipData& mip )
{
  sasFree( mip.data );
}

void FreeMips( MipDatas& mips )
{
  for( size_t i = 0; i < mips.size(); i++ )
  {
    FreeMip( mips[ i ] );    
  }
}

bool LoadImage( const char* path, sasTextureMipData& mip )
{
  bool ok = false;
  ILuint img = ilGenImage( );
  if( img )
  {
    ilBindImage( img );
    ilEnable( IL_ORIGIN_SET );
    ilOriginFunc( IL_ORIGIN_UPPER_LEFT );
    if( ilLoadImage( path ) )
    {
      ILint width = ilGetInteger( IL_IMAGE_WIDTH );
      ILint height = ilGetInteger( IL_IMAGE_HEIGHT );
      unsigned char* dataTransformed = (unsigned char*)sasMalloc( width*height*4 );
      ILuint nPixels = ilCopyPixels( 0, 0, 0, width, height, 1, IL_BGRA, IL_UNSIGNED_BYTE, dataTransformed );
      if( nPixels )
      {
        mip.width = width;
        mip.height = height;
        mip.format = sasPixelFormat::BGRA_U8;
        mip.data = dataTransformed;        
        mip.dataSize = width*height*4 ;
        ok = true;
      }
      else 
      {
        sasFree( dataTransformed );
      }
    }
    ilDeleteImage( img );
  } 

  return ok;
}

bool isPowerOf2( uint32_t v )
{
  if( v == 1 )
    return true;

  if( v == 0 )
    return false;

  uint32_t potRef = 1;
  while( potRef <= v )
  {
    potRef *= 2;
    if( v == potRef )
      return true;
  }

  return false;
}

uint32_t getMaxMipLevels( uint32_t v )
{
  float size = static_cast< float >( v );
  uint32_t levels = 1;
  while( size > 4.f )
  {
    size /= 2.f;
    float frac = 0.f;
    modff( size, &frac );
    frac = size - frac;
    if( frac != 0.f )
      return levels;

    levels++;
  }
  return levels;
}

bool CompressData( const sasTextureMipData& mip, MipDatas& dstDatas, bool hasAlphaChannel  )
{
  bool ok = true;

  InputOptions inputOptions;
  inputOptions.setTextureLayout( TextureType_2D, mip.width, mip.height );
  inputOptions.setMipmapData( mip.data, mip.width, mip.height );

  if( !isPowerOf2( mip.width ) || !isPowerOf2( mip.height ) )
  {
    int maxMipLevels = std::min< int >( getMaxMipLevels( mip.width ), getMaxMipLevels( mip.height ) );
    inputOptions.setMipmapGeneration( true, maxMipLevels );
  }
  else
  {
    inputOptions.setMipmapGeneration( true );
  }

  OutputOptions outputOptions;  
  outputOptions.setOutputHeader( false );

  CompressionOptions compressionOptions;
  compressionOptions.setFormat( hasAlphaChannel ? Format_DXT5 : Format_DXT1 );

  sasOutputHandler handler( dstDatas );
  outputOptions.setOutputHandler( &handler );

  Compressor compressor;
  ok = compressor.process(inputOptions, compressionOptions, outputOptions);

  if( ok )
  {
    for( size_t i = 0; i < dstDatas.size(); i++ )
    {
      dstDatas[ i ].format = hasAlphaChannel ? sasPixelFormat::DXT5 : sasPixelFormat::DXT1;
      dstDatas[ i ].pitch = ( dstDatas[ i ].dataSize / dstDatas[ i ].height ) * 4;
    }
  }

  return ok;
}

sasTextureLoader::sasTextureLoader()
{
  ilInit();
}

sasTextureLoader::~sasTextureLoader()
{
  ilShutDown();
}

bool loadMipsFromFile( const std::string& filename, MipDatas& mipDatas )
{
  sasFile dxtFile;
  if( !dxtFile.open( filename, sasFile::Read ) )
  {
    sasDebugOutput( "Couldn't open mips cache file '%s'; recreating.\n", filename.c_str() );
    return NULL;
  }

  uint32_t mipsCount;
  if( !dxtFile.read( mipsCount ) || mipsCount == 0 || mipsCount > 100 )
  {
    sasDebugOutput( "Couldn't read proper mips count from '%s'; recreating.\n", filename.c_str() );
    return NULL;
  }

  mipDatas.resize( mipsCount );
  bool result = true;
  for( uint32_t i = 0; i < mipsCount; ++i )
  {
    uint32_t tmpVar;
    result &= dxtFile.read( tmpVar ); mipDatas[ i ].width = tmpVar;
    result &= dxtFile.read( tmpVar ); mipDatas[ i ].height = tmpVar;
    result &= dxtFile.read( tmpVar ); mipDatas[ i ].format = ( sasPixelFormat::Enum )tmpVar;
    result &= dxtFile.read( tmpVar ); mipDatas[ i ].pitch = tmpVar;
    result &= dxtFile.read( tmpVar ); mipDatas[ i ].dataSize = tmpVar;
    mipDatas[ i ].data = ( unsigned char* )sasMalloc( mipDatas[ i ].dataSize );
    result &= ( dxtFile.read( mipDatas[ i ].data, mipDatas[ i ].dataSize ) == mipDatas[ i ].dataSize );
    
    if( !result )
    {
      break;
    }
  }
  
  if( !result )
  {
    sasDebugOutput( "Failed to load mips from file '%s'; corrupted. Recreating it.\n", filename.c_str() );
  }

  return result;
}

void saveMipsToFile(const MipDatas& mipDatas, const std::string& filename)
{
  sasFile dxtFile;
  if( !dxtFile.open( filename, sasFile::Write ) )
  {
    return;
  }
  
  uint32_t mipsCount = mipDatas.size();
  dxtFile.write(mipsCount);
  for( uint32_t i = 0; i < mipsCount; ++i )
  {
    dxtFile.write( ( uint32_t )mipDatas[ i ].width );
    dxtFile.write( ( uint32_t )mipDatas[ i ].height );
    dxtFile.write( ( uint32_t )mipDatas[ i ].format );
    dxtFile.write( ( uint32_t )mipDatas[ i ].pitch );
    dxtFile.write( ( uint32_t )mipDatas[ i ].dataSize );
    dxtFile.write( ( uint8_t* )mipDatas[ i ].data, mipDatas[ i ].dataSize );
  }
}

sasTextureResource* sasTextureLoader::loadTexture2d( const char* path )
{
  sasTextureResource* result = 0;
  sasTextureMipData srcData;

  std::string dxtCacheFile = path;
  dxtCacheFile += ".dxt";
  sasFile dxtFile;

  MipDatas mipDatas;
  bool r = loadMipsFromFile( dxtCacheFile, mipDatas );

  if( !r ) //need to generate mipDatas from source image if no cached dxt version
  {
    //search for alpha hint
    bool hasAlphaChannel = false;
    std::string key ("_ab_");
    size_t found;
    std::string res = path;
    found = res.rfind( key );
    if( found != std::string::npos )
    {
      hasAlphaChannel = true;
    }

    if( LoadImage( path, srcData ) )
    {
      if( CompressData( srcData, mipDatas, hasAlphaChannel ) )
      {
        saveMipsToFile( mipDatas, dxtCacheFile );
        r = true;
      }
    }
    FreeMip( srcData );
  }

  if( !r )
  {
    FreeMips( mipDatas );
    sasDebugOutput("Could not load mip data from file '%s', texture creation failed...\n", path );
    return NULL;
  }

  sasTextureId texId = sasHasher::fnv_64_str( path );

  sasTexture* texture = sasNew sasTexture( &mipDatas[ 0 ], mipDatas.size(), 1 );            
  sasTextureResource* texRsc = sasNew sasTextureResource( texId, texture, path );

  return texRsc;
}
  

sasResource* sasTextureLoader::load( const char* path )
{
  const char* ext = sasPathGetExt( path );

  bool isTexArray = false;
  if( strcmp( ext, "sasTexArray" ) == 0 )
  {
    isTexArray = true;
  }

  if( isTexArray )
  {
    return loadTextureArray2d( path );
  }
  else
  {
    return loadTexture2d( path );
  }

}

void sasTextureLoader::loadTexArrayDesc( const char* path, TexArrayDesc& texArrayDesc )
{
  sasFile taFile;
  if( !taFile.open( path, sasFile::ReadText ) )
  {
    sasDebugOutput("Couldn't open texture array descriptor file '%s'\n", path );
    return;
  }

  std::string tad;
  taFile.read( tad );

  if( tad.size() == 0 )
  {
    sasDebugOutput("texture array descriptor file '%s' is empty\n", path );
    return;
  }

  size_t i = 0;
  while( i < tad.size() )
  {
    std::string currentStr;
    for( ; i < tad.size(); i++ )
    {
      if( tad[ i ] == ',' )
      {
        i++;
        break;
      }

      if( tad[ i ] == '\n' )
        continue;

      currentStr += tad[ i ];
    }
    if( currentStr.size() )
      texArrayDesc.push_back( currentStr );
  }
  texArrayDesc.pop_back();
}


sasTextureResource* sasTextureLoader::loadTextureArray2d( const char* path )
{
  TexArrayDesc texArrayDesc;
  loadTexArrayDesc( path, texArrayDesc );

  uint32_t numTextures = texArrayDesc.size();

  sasTextureResource* result = 0;
  sasTextureMipData* srcData = new sasTextureMipData[ numTextures ];

  MipDatas* mipDatas = new MipDatas[ numTextures ];

  for( uint32_t i = 0; i < numTextures; i++ )
  {
    std::string texFilename = sasResourceMng::singletonPtr()->resourcePath();
    texFilename += texArrayDesc[ i ];
    
    std::string dxtCacheFile = texFilename;
    dxtCacheFile += ".dxt";

    sasFile dxtFile;

    bool r = loadMipsFromFile( dxtCacheFile, mipDatas[ i ] );

    if( !r ) //need to generate mipDatas from source image if no cached dxt version
    {
      //search for alpha hint
      bool hasAlphaChannel = false;
      std::string key ("_ab_");
      size_t found;
      std::string res = texFilename.c_str();
      found = res.rfind( key );
      if( found != std::string::npos )
      {
        hasAlphaChannel = true;
      }

      if( LoadImage( texFilename.c_str(), srcData[ i ] ) )
      {
        if( CompressData( srcData[ i ], mipDatas[ i ], hasAlphaChannel ) )
        {
          saveMipsToFile( mipDatas[ i ], dxtCacheFile );
          r = true;
        }
      }
      FreeMip( srcData[ i ] );
    }

    if( i > 0 )
    {
      if( mipDatas[ i ].size() != mipDatas[ 0 ].size() )
      {
        sasDebugOutput("loadTextureArray2d: mismatch in resolution and mip count in between textures in array, must be identical...\n" );
        r = false;
      }

      if( r && ( ( mipDatas[ i ] )[ 0 ].format != ( mipDatas[ 0 ] )[ 0 ].format ) )
      {
        sasDebugOutput("loadTextureArray2d: mismatch in format in between textures in array, must be identical...\n" );
        r = false;
      }
    }

    if( !r )
    {
      for( uint32_t j = 0; j < i; j++ )
        FreeMips( mipDatas[ j ] );

      sasDebugOutput("Could not load mip data from file '%s', texture array creation failed...\n", path );
      delete [] mipDatas;
      delete [] srcData;
      return NULL;
    }
  }
  
  uint32_t numMipsPerTex = mipDatas[ 0 ].size();
  MipDatas finalMipDataArray;
  finalMipDataArray.resize( numMipsPerTex * numTextures );
  for( uint32_t a = 0; a < numTextures; a++ )
  {
    uint32_t offset = a * numMipsPerTex;
    for( uint32_t m = 0; m < numMipsPerTex; m++ )
    {
      finalMipDataArray[ offset + m ] = ( mipDatas[ a ] )[ m ];
    }
  }

  sasTextureId texId = sasHasher::fnv_64_str( path );

  sasTexture* texture = sasNew sasTexture( &finalMipDataArray[ 0 ], mipDatas[ 0 ].size(), numTextures );            
  sasTextureResource* texRsc = sasNew sasTextureResource( texId, texture, path );

  delete [] mipDatas;
  delete [] srcData;

  return texRsc;
}