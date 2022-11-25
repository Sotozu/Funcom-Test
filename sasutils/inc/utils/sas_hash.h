#pragma once

namespace sasHasher
{
  /*******************************************
    public domain hash functions
    http://isthe.com/chongo/tech/comp/fnv/
  *******************************************/
  
  #define FNV1_32_PRIME	(0x01000193UL)
  #define FNV1_32_INIT	(0x811c9dc5UL)
  #define FNV1A_32_INIT ((uint32_t)2166136261)
  #define FNV1_64_INIT	(0xcbf29ce484222325ULL)
  #define FNV1_64_PRIME	(0x100000001b3ULL)

  uint32_t fnv_32_buf(const void *buf, size_t len, uint32_t hval = FNV1_32_INIT);
  uint32_t fnva_32_buf(const void *buf, size_t len, uint32_t hval = FNV1A_32_INIT);

  uint32_t fnv_32_str(const char *str, uint32_t hval = FNV1_32_INIT);

  uint64_t fnv_64_buf(const void *buf, size_t len, uint64_t hval = FNV1_64_INIT);

  uint64_t fnv_64_str(const char *str, uint64_t hval = FNV1_64_INIT);

  void initializeCrcTable();
  uint32_t crc_32_buf(const void *buf, size_t iDataLength);

  template<typename T>
  struct BufferHashCompare
  {
    enum 
    {              
      bucket_size = 4,  
      min_buckets = 8
    }; 

    size_t operator () (const T& buffer) const
    {
      return sasHasher::fnv_32_buf(&buffer, sizeof(T));
    }

    bool operator () (const T& b1, const T& b2) const
    {
      return memcmp(&b1, &b2, sizeof(T)) < 0 ? true : false;
    }
  };
} //Hash


