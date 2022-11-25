#pragma once 

#include "sm_common.h"

smALIGN16_PRE struct smVec4 
{
public:
  //! ctor/dtor
  smVec4()                                              { smALIGNED(this); }
  smVec4( float x, float y, float z, float w )          { smALIGNED(this); set(x, y, z, w); }
  smVec4( float a )                                     { smALIGNED(this); R128 = _mm_set_ps1(a); }  
  smVec4( const __m128& r128 )                          { smALIGNED(this); R128 = r128; }

  //! methods

  void set( float x, float y, float z, float w )        { X = x; Y = y; Z = z; W = w; }  
  void set( float x, float y, float z )                 { X = x; Y = y; Z = z; }  

  //! operators
  float    operator [] ( size_t i ) const               { return v[i]; }  
  float&   operator [] ( size_t i )                     { return v[i]; }  
  smVec4&  operator *= ( float a )                      { R128 = _mm_mul_ps(R128, _mm_set_ps1(a)); return *this; }
  smVec4&  operator /= ( float a )                      { R128 = _mm_div_ps(R128, _mm_set_ps1(a)); return *this; }
  smVec4&  operator += ( float a )                      { R128 = _mm_add_ps(R128, _mm_set_ps1(a)); return *this; }
  smVec4&  operator -= ( float a )                      { R128 = _mm_sub_ps(R128, _mm_set_ps1(a)); return *this; }
  smVec4&  operator *= ( const smVec4& v )              { R128 = _mm_mul_ps(R128, v.R128); return *this; }
  smVec4&  operator /= ( const smVec4& v )              { R128 = _mm_div_ps(R128, v.R128); return *this; }
  smVec4&  operator += ( const smVec4& v )              { R128 = _mm_add_ps(R128, v.R128); return *this; }
  smVec4&  operator -= ( const smVec4& v )              { R128 = _mm_sub_ps(R128, v.R128); return *this; }
  
  //! members
  union 
  {
    float v[4];
    struct{float X, Y, Z, W;};
    __m128 R128;
  };
} smALIGN16_POST;

smALIGN16_PRE struct smVec3 : public smVec4 
{
public:
  //! ctor/dtor
  smVec3()                                            { smALIGNED(this); }
  //explicitly set the w component to 0
  smVec3(float x, float y, float z) { smALIGNED(this); set(x, y, z);}
  smVec3( float a )                                   { smALIGNED(this); R128 = _mm_set_ps1(a); }
  smVec3( const smVec3& v )                           { smALIGNED(this); R128 = v.R128; }
  smVec3( const smVec4& v )                           { smALIGNED(this); R128 = v.R128; }
  smVec3( __m128& r128 )                              { smALIGNED(this); R128 = r128; }

  //! methods
  void set( float x, float y, float z )               { X = x; Y = y; Z = z; }

  //! operators
  smVec3& operator = ( const smVec3& v )              { R128 = v.R128; return *this; }
  //NEW - Alex Soto
  float magnitudeSquared()             { return (X * X + Y * Y + Z * Z); }
  //NEW - Alex Soto
  float magnitude()                    { return static_cast<float>(sqrt(static_cast<double>(magnitudeSquared()))); }
} smALIGN16_POST;

smALIGN16_PRE struct smVec2 : public smVec4 
{
public:
  //! ctor/dtor
  smVec2()                                            { smALIGNED(this); }
  smVec2( float x, float y )                          { smALIGNED(this); set(x, y ); }
  smVec2( float a )                                   { smALIGNED(this); R128 = _mm_set_ps1(a); }
  smVec2( const smVec2& v )                           { smALIGNED(this); R128 = v.R128; }
  smVec2( __m128& r128 )                              { smALIGNED(this); R128 = r128; }

  //! methods
  void set( float x, float y )                        { X = x; Y = y; }

  //! operators
  smVec2& operator = ( const smVec2& v )              { R128 = v.R128; return *this; }
} smALIGN16_POST;

smINLINE smVec4  operator * ( const smVec4& v, float a )            { return _mm_mul_ps(v.R128, _mm_set_ps1(a)); }
smINLINE smVec4  operator / ( const smVec4& v, float a )            { return _mm_div_ps(v.R128, _mm_set_ps1(a)); }
smINLINE smVec4  operator + ( const smVec4& v, float a )            { return _mm_add_ps(v.R128, _mm_set_ps1(a)); }
smINLINE smVec4  operator - ( const smVec4& v, float a )            { return _mm_sub_ps(v.R128, _mm_set_ps1(a)); }
smINLINE smVec4  operator * ( float a, const smVec4& v )            { return _mm_mul_ps(v.R128, _mm_set_ps1(a)); }
smINLINE smVec4  operator / ( float a, const smVec4& v )            { return _mm_div_ps(v.R128, _mm_set_ps1(a)); }
smINLINE smVec4  operator + ( float a, const smVec4& v )            { return _mm_add_ps(v.R128, _mm_set_ps1(a)); }
smINLINE smVec4  operator - ( float a, const smVec4& v )            { return _mm_sub_ps(v.R128, _mm_set_ps1(a)); }
smINLINE smVec4  operator * ( const smVec4& v1, const smVec4& v2 )  { return _mm_mul_ps(v1.R128, v2.R128); }
smINLINE smVec4  operator / ( const smVec4& v1, const smVec4& v2 )  { return _mm_div_ps(v1.R128, v2.R128); }
smINLINE smVec4  operator + ( const smVec4& v1, const smVec4& v2 )  { return _mm_add_ps(v1.R128, v2.R128); }
smINLINE smVec4  operator - ( const smVec4& v1, const smVec4& v2 )  { return _mm_sub_ps(v1.R128, v2.R128); }
smINLINE bool operator ==( const smVec4& v1, const smVec4& v2 )    { return (_mm_movemask_ps(_mm_cmpeq_ps(v1.R128, v2.R128)) == 0xf); }
smINLINE bool operator !=( const smVec4& v1, const smVec4& v2 )    { return (_mm_movemask_ps(_mm_cmpeq_ps(v1.R128, v2.R128)) != 0xf); }
smINLINE bool operator >=( const smVec4& v1, const smVec4& v2 )    { return (_mm_movemask_ps(_mm_cmpge_ps(v1.R128, v2.R128)) == 0xf); }
smINLINE bool operator <=( const smVec4& v1, const smVec4& v2 )    { return (_mm_movemask_ps(_mm_cmple_ps(v1.R128, v2.R128)) != 0xf); }
smINLINE bool operator > ( const smVec4& v1, const smVec4& v2 )    { return (_mm_movemask_ps(_mm_cmpgt_ps(v1.R128, v2.R128)) == 0xf); }
smINLINE bool operator < ( const smVec4& v1, const smVec4& v2 )    { return (_mm_movemask_ps(_mm_cmplt_ps(v1.R128, v2.R128)) == 0xf); }

smINLINE void    smAdd( const smVec4& a, const smVec4& b, smVec4& r ) { r.R128 = _mm_add_ps(a.R128, b.R128); }
smINLINE void    smAdd3( const smVec4& a, const smVec4& b, smVec4& r ) { r.X=a.X+b.X; r.Y=a.Y+b.Y; r.Z=a.Z+b.Z; }

smINLINE void    smSub( const smVec4& a, const smVec4& b, smVec4& r ) { r.R128 = _mm_sub_ps(a.R128, b.R128); }
smINLINE void    smSub3( const smVec4& a, const smVec4& b, smVec4& r ) { r.X=a.X-b.X; r.Y=a.Y-b.Y; r.Z=a.Z-b.Z; }

smINLINE void    smMul( const smVec4& a, const smVec4& b, smVec4& r ) { r.R128 = _mm_mul_ps(a.R128, b.R128); }

//NEW - Alex Soto
//SIMD
smINLINE void    smMul3( const smVec4& a, const smVec4& b, smVec4& r ) { r.R128 = _mm_mul_ps(a.R128, b.R128); }

smINLINE void    smDiv( const smVec4& a, float b, smVec4& r )         { r.R128 = _mm_div_ps(a.R128, _mm_set_ps1(b) ); }
smINLINE void    smDiv3( const smVec4& a, float b, smVec4& r )        { r.X /= b; r.Y /= b; r.Z /= b; }

smINLINE void    smInverse( const smVec4& v, smVec4& r )              { r.X = -v.X; r.Y = -v.Y; r.Z = -v.Z; r.W = -v.W;  }
smINLINE void    smInverse3( const smVec4& v, smVec4& r )             { r.X = -v.X; r.Y = -v.Y; r.Z = -v.Z; }

float smDot( const smVec4& v1, const smVec4& v2 );
float smDot3( const smVec4& v1, const smVec4& v2 );

smINLINE float   smLength( const smVec4& v )                          { return sqrtf(smDot(v, v)); }
smINLINE float   smLength3( const smVec4& v )                         { return sqrtf(smDot3(v, v)); }
smINLINE float   smLengthSq( const smVec4& v )                        { return smDot(v, v); }
smINLINE float   smLengthSq3( const smVec4& v )                       { return smDot3( v, v ); }

smINLINE void    smNormalize( smVec4& v )                             { float l = smLength(v);  v = (l ? (v / l) : v); }
smINLINE void    smNormalize3( const smVec4& v, smVec4& r )           { float l = smLength3(v); smMul3( v, 1.f / l, r ); }

smVec4  smCross( const smVec4& v1, const smVec4& v2 );       
void    smCross( const smVec4& v1, const smVec4& v2, smVec4& r );

smINLINE smVec4  smMin( const smVec4& v1, const smVec4& v2 )           { return _mm_min_ps(v1.R128, v2.R128); }
smINLINE void    smMin( const smVec4& v1, const smVec4& v2, smVec4& r ) { r.R128  = _mm_min_ps(v1.R128, v2.R128); }
smINLINE void    smMin3( const smVec4& v1, const smVec4& v2, smVec4& r ){ r.X = smMin(v1.X, v2.X); r.Y = smMin(v1.Y, v2.Y); r.Z = smMin(v1.Z, v2.Z); }

smINLINE smVec4  smMax( const smVec4& v1, const smVec4& v2)           { return _mm_max_ps(v1.R128, v2.R128); }
smINLINE void    smMax( const smVec4& v1, const smVec4& v2, smVec4& r ) { r.R128  = _mm_max_ps(v1.R128, v2.R128); }
smINLINE void    smMax3( const smVec4& v1, const smVec4& v2, smVec4& r ){ r.X = smMax(v1.X, v2.X); r.Y = smMax(v1.Y, v2.Y); r.Z = smMax(v1.Z, v2.Z); }

smINLINE smVec4  smLerp( const smVec4& v1, const smVec4& v2, float c) { return _mm_add_ps(_mm_mul_ps(v1.R128, _mm_set_ps1(1.f - c)),  _mm_mul_ps(v2.R128, _mm_set_ps1(c))); }
smINLINE smVec2  smLerp( const smVec2& v1, const smVec2& v2, float c) { return _mm_add_ps(_mm_mul_ps(v1.R128, _mm_set_ps1(1.f - c)),  _mm_mul_ps(v2.R128, _mm_set_ps1(c))); }
smINLINE float   smLerp( float v1, float v2, float c) { return v1 + c * ( v2 - v1 ); }

