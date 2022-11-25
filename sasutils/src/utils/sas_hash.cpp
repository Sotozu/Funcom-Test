#include "su_pch.h"
#include "utils/sas_hash.h"

namespace sasHasher
{
  uint32_t fnv_32_buf(const void *buf, size_t len, uint32_t hval)
  {
    unsigned char *bp = (unsigned char *)buf;	/* start of buffer */
    unsigned char *be = bp + len;		/* beyond end of buffer */

    /*
    * FNV-1 hash each octet in the buffer
    */
    while (bp < be) 
    {
      /* multiply by the 32 bit FNV magic prime mod 2^32 */
      hval *= FNV1_32_PRIME;

      /* xor the bottom with the current octet */
      hval ^= (uint32_t)*bp++;
    }
    /* return our new hash value */
    return hval;
  }

  uint32_t fnv_32_str(const char *str, uint32_t hval)
  {
    unsigned char *s = (unsigned char *)str;	/* unsigned string */

    /*
    * FNV-1 hash each octet in the buffer
    */
    while (*s) 
    {
      /* multiply by the 32 bit FNV magic prime mod 2^32 */
      hval *= FNV1_32_PRIME;

      /* xor the bottom with the current octet */
      hval ^= (uint32_t)*s++;
    }
    /* return our new hash value */
    return hval;
  }

  uint32_t fnva_32_buf(const void *buf, size_t len, uint32_t /*hval*/)
  {
//     unsigned char *bp = (unsigned char *)buf;	/* start of buffer */
//     unsigned char *be = bp + len;		/* beyond end of buffer */
// 
//     /*
//     * FNV-1 hash each octet in the buffer
//     */
//     while (bp < be) 
//     {
//       hval ^= (uint32_t)*bp++;
// 
// 
//       /* multiply by the 32 bit FNV magic prime mod 2^32 */
// #ifdef OPT_TEST
//       hval *= FNV1_32_PRIME;
// #else
//       hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
// #endif
// 
//     }
//     /* return our new hash value */
//     return hval;

    const UINT PRIME = 1607;

    UINT hash32 = 2166136261;
    const char *p = (char*)buf;

    for(; len >= sizeof(DWORD); len -= sizeof(DWORD), p += sizeof(DWORD)) 
    {
      hash32 = (hash32 ^ *(DWORD *)p) * PRIME;
    }
    if (len & sizeof(WORD)) {
      hash32 = (hash32 ^ *(WORD*)p) * PRIME;
      p += sizeof(WORD);
    }
    if (len & 1) 
      hash32 = (hash32 ^ *p) * PRIME;

    return hash32 ^ (hash32 >> 16);
  }

  uint64_t fnv_64_buf(const void *buf, size_t len, uint64_t hval)
  {
    unsigned char *bp = (unsigned char *)buf;	/* start of buffer */
    unsigned char *be = bp + len;		/* beyond end of buffer */

    /*
    * FNV-1 hash each octet of the buffer
    */
    while (bp < be) 
    {
      /* multiply by the 64 bit FNV magic prime mod 2^64 */
      hval *= FNV1_64_PRIME;

      /* xor the bottom with the current octet */
      hval ^= (uint64_t)*bp++;
    }
    return hval;
  }

  uint64_t fnv_64_str(const char *str, uint64_t hval)
  {
    unsigned char *s = (unsigned char *)str;	/* unsigned string */

    /*
    * FNV-1 hash each octet of the string
    */
    while (*s) 
    {
      /* multiply by the 64 bit FNV magic prime mod 2^64 */
      hval *= FNV1_64_PRIME;

      /* xor the bottom with the current octet */
      hval ^= (uint64_t)*s++;
    }
    return hval;
  }

  static uint32_t sCrcTable[ 256 ];

  unsigned int crcReflect(unsigned int iReflect, const char cChar)
  {
    unsigned int iValue = 0;

    // Swap bit 0 for bit 7, bit 1 For bit 6, etc....
    for(int iPos = 1; iPos < (cChar + 1); iPos++)
    {
      if(iReflect & 1)
      {
        iValue |= (1 << (cChar - iPos));
      }
      iReflect >>= 1;
    }

    return iValue;
  }

  void initializeCrcTable()
  {
    //0x04C11DB7 is the official polynomial used by PKZip, WinZip and Ethernet.
    unsigned int iPolynomial = 0x04C11DB7;

    memset( &sCrcTable, 0, sizeof( sCrcTable ) );

    // 256 values representing ASCII character codes.
    for(int iCodes = 0; iCodes <= 0xFF; iCodes++)
    {
      sCrcTable[iCodes] = crcReflect(iCodes, 8) << 24;

      for(int iPos = 0; iPos < 8; iPos++)
      {
        sCrcTable[iCodes] = (sCrcTable[iCodes] << 1)
          ^ ((sCrcTable[iCodes] & (1 << 31)) ? iPolynomial : 0);
      }

      sCrcTable[iCodes] = crcReflect(sCrcTable[iCodes], 32);
    }
  }

  uint32_t crc_32_buf(const void *sData, size_t iDataLength)
  {
    const unsigned char* data = static_cast< const unsigned char* >( sData );
    unsigned int iCRC = 0xffffffff; 

    while(iDataLength--)
    {
      iCRC = (iCRC >> 8) ^ sCrcTable[(iCRC & 0xFF) ^ *data++];
    }

    return(iCRC ^ 0xffffffff);
  }

} //Hash


