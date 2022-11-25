#pragma once
#include "sys/sas_scratchmem.h"

template< class T > void radixSortAscending( int numItems, T* items, int keyOffset, int keySize )
{
  T* temp = ( T* )gScratchMemMng->allocateScratchMemory( numItems * sizeof( T ) );
  T* src = items;
  T* dst = temp;

  for( int i = 0; i < keySize; i++ )
  {
    unsigned count[ 256 ];
    memset( count, 0, sizeof( count ) );
    unsigned char* keyb = ( ( unsigned char* ) src ) + keyOffset + i;
    for( int j = 0; j < numItems; j++ )
    {
      count[ keyb[ 0 ] ]++;
      keyb += sizeof( T );
    }
    unsigned offset = 0;
    for( int j = 0; j < 256; j++ )
    {
      unsigned k = count[ j ];
      count[ j ] = offset;
      offset += k;
    }
    keyb = ( (unsigned char* ) src ) + keyOffset + i;
    for( int j = 0; j < numItems; j++ )
    {
      dst[ count[ keyb[ 0 ] ]++ ] = src[ j ];
      keyb += sizeof( T );
    }
    std::swap( dst, src );
  }
  if( keySize & 1 )
  {
    memcpy( items, src, numItems * sizeof( T ) );
  }
  gScratchMemMng->freeScratchMemory( temp );
}