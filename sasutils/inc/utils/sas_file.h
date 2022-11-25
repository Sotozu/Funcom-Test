#pragma once 

class sasVirtualFile;

class sasFile
{
public:
  enum
  {
    Read = 1<<0,
    Write = 1<<1,
    ReadText = 1<<2,
    WriteText = 1<<3,
    AppendText = 1<<4,
  };

  sasFile();
  ~sasFile();

  static bool copyBinary( const std::string& srcFilename, const std::string& dstFilename );
  bool open( const std::string& filename, uint32_t flags ) { return open(filename.c_str(), flags); }
  bool open( const char* filename, uint32_t flags );
  size_t read( std::string& str );
  size_t read( uint8_t* buffer, size_t length );
  size_t read( sasVirtualFile* vFile, size_t size );
  size_t write(const uint8_t* buffer, size_t length );
  size_t getSize() const { return _size; }

  template<typename T>
  bool write(const T& data) { return write((uint8_t*)&data, sizeof(T)) == sizeof(T); }

  template<typename T>
  bool read(const T& data) { return read((uint8_t*)&data, sizeof(T)) == sizeof(T); }

  size_t writeStr( const char* str );
  size_t readStr( std::string& str );

  
private:
  bool _opened;
  FILE* _handle;
  size_t _size;

  sasMEM_OP( sasFile );
  sasNO_COPY( sasFile );
};