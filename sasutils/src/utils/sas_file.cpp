#include "su_pch.h"
#include "utils/sas_file.h"
#include "utils/sas_virtual_file.h"

sasFile::sasFile()
  : _opened(false)
  , _handle(NULL)
  , _size(0)
{
}

sasFile::~sasFile()
{
  if(_opened)
  {
    fclose(_handle);    
  }
}

bool sasFile::open(const char* filename, uint32_t flags)
{
  FILE* file;
  const char* mode = "rb";
  if( flags & sasFile::Write )
  {
    mode = "wb";
  }
  else if( flags & sasFile::WriteText )
  {
    mode = "w";
  }
  else if( flags & sasFile::AppendText )
  {
    mode = "a";
  }
  else if( flags & sasFile::ReadText )
  {
    mode = "r";
  }

  file = fopen(filename, mode);
  if( file == NULL )
  {
    return false;
  }
  
  _handle = file;
  _opened = true;
  
  fseek(_handle , 0 , SEEK_END);
  _size = ftell(_handle);
  rewind(_handle);
  
  return true;
}

size_t sasFile::read(std::string& str)
{
  str.resize(_size);
  return read((uint8_t*)str.c_str(), _size);
}

size_t sasFile::read(uint8_t* buffer, size_t length)
{
  if(_opened)
  {
    return fread(buffer, 1, length, _handle);      
  }
  
  return 0;
}

size_t sasFile::read( sasVirtualFile* vFile, size_t size )
{
  sasASSERT( vFile );
  uint8_t* destPtr = vFile->_data;
  destPtr += vFile->currentSize();
  return read( destPtr, size );
}

size_t sasFile::write(const uint8_t* buffer, size_t length)
{
  if(_opened)
  {
    size_t bytesWritten = fwrite(buffer, 1, length, _handle);
    _size += bytesWritten;
    return bytesWritten;
  }
  
  return 0;
}

bool sasFile::copyBinary( const std::string& srcFilename, const std::string& dstFilename )
{
  sasFile srcFile;
  if( !srcFile.open( srcFilename.c_str(), sasFile::Read ) )
  {
    sasDebugOutput( "sasFile::copyBinary - could not open source file: %s\n", srcFilename.c_str() );
    return false;
  }

  sasFile destFile;
  if( !destFile.open( dstFilename.c_str(), sasFile::Write ) )
  {
    sasDebugOutput( "sasFile::copyBinary - could not open destination file for write: %s\n", dstFilename.c_str() );
    return false;
  }

  uint8_t* data = new uint8_t[ srcFile.getSize() ];
  srcFile.read( data, srcFile.getSize() );

  destFile.write( data, srcFile.getSize() );
  delete [] data;
  return true;
}

size_t sasFile::writeStr( const char* str )
{
  sasASSERT( str );
  uint32_t strLen = strlen( str );
  write( strLen );
  size_t bytesWritten = 4;
  bytesWritten += write( reinterpret_cast< const uint8_t* >( str ), strLen );
  return bytesWritten;
}

size_t sasFile::readStr( std::string& str )
{
  uint32_t strLength = 0;
  read( strLength );
  sasASSERT( ( strLength > 0 ) && ( strLength < getSize() ) );
  char* tempStr = new char[ strLength + 1 ];
  size_t bytesRead = read( reinterpret_cast< uint8_t* >( tempStr ), strLength );
  tempStr[ strLength ] = 0;
  str = tempStr;
  delete [] tempStr;
  return bytesRead + 4;
}
