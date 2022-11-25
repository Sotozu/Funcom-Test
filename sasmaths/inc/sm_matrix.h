#pragma once 

#include "sm_common.h"
#include "sm_vector.h"

struct smEulerAngles;

smALIGN16_PRE struct smMat44
{
  //! ctor/dtor
  smMat44() { smALIGNED(this); }

  //! methods
  void setIdentity();
  void transpose();

  void set( float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33 );

  void set( float* ptr );

  template< size_t i >
  void getColumn( smVec4& c ) const 
  {
    smSTATIC_ASSERT( i < 4 );
    c.set( L0[i], L1[i], L2[i], L3[i] );
  }

  template< size_t i >
  void getColumn3( smVec4& c ) const 
  {
    smSTATIC_ASSERT( i < 4 );
    c.set( L0[i], L1[i], L2[i] );
  }

  template< size_t i >
  void setColumn3( const smVec4& c ) 
  {
    smSTATIC_ASSERT( i < 4 );
    L0[i] = c.X; L1[i] = c.Y; L2[i] = c.Z;
  }

  template< size_t i >
  void setColumn4( const smVec4& c ) 
  {
    smSTATIC_ASSERT( i < 4 );    
    L0[i] = c.X; L1[i] = c.Y; L2[i] = c.Z; L3[i] = c.W;
  }

  //! members
  smVec4 L0, L1, L2, L3;

} smALIGN16_POST;

void smMul( const smMat44& a, const smMat44& b, smMat44& r );
void smMul3( const smMat44& a, const smMat44& b, smMat44& r );
void smMul( const smMat44& a, const smVec4& b, smVec4& r );
void smMul3( const smMat44& a, const smVec4& b, smVec4& r );

bool smInverse( const smMat44& a, smMat44& r );
bool smInverse3( const smMat44& a, smMat44& r );

void smSetTranslation( const smVec4& tr, smMat44& m );
void smSetTranslation( float trX, float trY, float trZ, smMat44& m );
void smGetTranslation( const smMat44& m, smVec4& tr );

void smGetRotIMatrix( float ang, smMat44& r );
void smGetRotJMatrix( float ang, smMat44& r );
void smGetRotKMatrix( float ang, smMat44& r );

void smGetXFormTRSMatrix( const smVec4& tr, const smEulerAngles& rot, float sc, smMat44& r );
void smGetXFormTRSMatrix( float trX, float trY, float trZ, const smEulerAngles& rot, float sc, smMat44& r );
void smGetXFormTRSMatrix( float trX, float trY, float trZ, const smEulerAngles& rot, float scX, float scY, float scZ, smMat44& r );

void smGetScalingMatrix( float sX, float sY, float sZ, smMat44& r );