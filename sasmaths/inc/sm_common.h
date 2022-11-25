#pragma once 

#include <math.h>
#include <xmmintrin.h>

// ----------------------------------------------------------------------------

#ifdef _MSC_VER
  #define smALIGN16_PRE             __declspec( align(16) )
  #define smALIGN16_POST
#else
  #define smALIGN16_PRE
  #define smALIGN16_POST           __attribute__ ((aligned (16)))
#endif

#ifdef _MSC_VER
  #define smASSERT(C)           do{ if( !(C) ) { __asm int 3  } } while(0)
#else
  #define smASSERT(C)           do{ if( !(C) ) { __asm {int 3}  } } while(0)
#endif

#ifdef _MSV_VER
  #define smSTATIC_ASSERT(C)    static_assert(C, #C)
#else
  #define smSTATIC_ASSERT(C)
#endif

#define smALIGNED(P)          smASSERT( ( 0xf & size_t(P) ) == 0 )
#define smINLINE              inline 

// ----------------------------------------------------------------------------

#define smPI                  (3.1415926535897932384f)
#define sm2PI                 (2.f*smPI)

#define smSWIZZLE_YXWZ(a)     _mm_shuffle_ps(a, a, _MM_SHUFFLE(2, 3, 0, 1))
#define smSWIZZLE_ZWXY(a)     _mm_shuffle_ps(a, a, _MM_SHUFFLE(1, 0, 3, 2))
#define smSWIZZLE_YZXW(a)     _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1))
#define smSWIZZLE_ZXYW(a)     _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2))

// ----------------------------------------------------------------------------
//  Basic
// ----------------------------------------------------------------------------

template< typename T >
smINLINE T smMin( const T& a, const T& b )
{
  return a < b ? a : b;
}

template< typename T >
smINLINE T smMax( const T& a, const T& b )
{
  return a > b ? a : b;
}

template< typename T >
smINLINE T smAbs( const T& a )
{
  return a < 0 ? -a : a;
}

template< typename T >
smINLINE T smClamp( T min, T max, T value )
{
  if( value < min ) return min;
  if( value > max ) return max;
  return value;
};

// ----------------------------------------------------------------------------
//  Trigonometry
// ----------------------------------------------------------------------------

smINLINE float smNormalizeAngle( float a )
{
  while( a >=360.f )  a -= 360.f;
  while( a < 0.f )    a += 360.f;
  return a;
}

smINLINE float smClamp( float a, float min, float max )
{
  if( a > max )  a = max;
  if( a < min )    a = min;
  return a;
}

smINLINE float smDeg2Rad( float a ) 
{
  return ( a * smPI ) / 180.f;
}

smINLINE float smRad2Deg( float a ) 
{
  return ( a * 180.f ) / smPI;
}

smINLINE void smCosSin( float ang, float&c, float& s ) 
{
  float a = smDeg2Rad( ang );
  c = cosf( a );
  s = sinf( a );
}

smINLINE float smTan( float deg )
{
  return tanf( smDeg2Rad(deg) );
}

smINLINE float smATan( float deg )
{
  return atanf( smDeg2Rad(deg) );
}
