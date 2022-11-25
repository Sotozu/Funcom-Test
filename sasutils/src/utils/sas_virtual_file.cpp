#include "su_pch.h"
#include "utils/sas_virtual_file.h"

sasVirtualFile::sasVirtualFile()
  : _size( 0 )
  , _currentOffset( 0 )
  , _data( nullptr )
{
}

sasVirtualFile::~sasVirtualFile()
{
  if( _data != nullptr )
  {
    delete [] _data;
  }
}

bool sasVirtualFile::open( uint32_t initSize )
{
  _size = initSize;
  _currentOffset = 0;
  _data = new uint8_t[ initSize ];
  
  return true;
}

size_t sasVirtualFile::write( const uint8_t* buffer, size_t length )
{
  if( _data != nullptr )
  {
    if( ( _currentOffset + length ) > _size )
    {
      uint32_t newSize;
      if( length > _size )
        newSize = length * 2;
      else
        newSize = _size * 2;
      uint8_t* newDataBlock = new uint8_t[ newSize ];
      memcpy( newDataBlock, _data, _size );
      delete [] _data;
      _data = newDataBlock;
      _size = newSize;
    }

    uint8_t* currentPtr = &( _data[ _currentOffset ] );
    memcpy( currentPtr, buffer, length );

    _currentOffset += length;
    return length;
  }
  
  return 0;
}

size_t sasVirtualFile::writeStr( const char* str )
{
  sasASSERT( str );
  uint32_t strLen = strlen( str );
  write( strLen );
  size_t bytesWritten = 4;
  bytesWritten += write( reinterpret_cast< const uint8_t* >( str ), strLen );
  return bytesWritten;
}

size_t sasVirtualFile::read( uint8_t* buffer, size_t length )
{
  if( _data && ( ( _currentOffset + length ) <= _size ) )
  {
    uint8_t* currentPtr = &( _data[ _currentOffset ] );
    memcpy( buffer, currentPtr, length );

    _currentOffset += length;
    return length;     
  }

  return 0;
}

size_t sasVirtualFile::readStr( std::string& str )
{
  uint32_t strLength = 0;
  read( strLength );
  sasASSERT( ( strLength > 0 ) && ( strLength < ( totalSize() - currentSize() ) ) );
  char* tempStr = new char[ strLength + 1 ];
  size_t bytesRead = read( reinterpret_cast< uint8_t* >( tempStr ), strLength );
  tempStr[ strLength ] = 0;
  str = tempStr;
  delete [] tempStr;
  return bytesRead + 4;
}

