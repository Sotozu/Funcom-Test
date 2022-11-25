#pragma once 

class sasVirtualFile
{
  friend class sasFile;
public:
  sasVirtualFile();
  ~sasVirtualFile();

  bool open( uint32_t initSize );

  size_t write(const uint8_t* buffer, size_t length);
  size_t read( uint8_t* buffer, size_t length );

  size_t totalSize() const { return _size; }
  size_t currentSize() const { return _currentOffset; }

  const uint8_t* data() const { return _data; }

  template<typename T>
  bool write(const T& data) { return write((uint8_t*)&data, sizeof(T)) == sizeof(T); }

  template<typename T>
  bool read(const T& data) { return read((uint8_t*)&data, sizeof(T)) == sizeof(T); }

  size_t writeStr( const char* str );
  size_t readStr( std::string& str );
  
private:
  uint8_t*  _data;
  uint32_t  _currentOffset;
  size_t    _size;

  sasMEM_OP( sasVirtualFile );
  sasNO_COPY( sasVirtualFile );
};