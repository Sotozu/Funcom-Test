#include "sm_matrix.h"
#include "sm_vector.h"
#include "sm_euler.h"
#include <algorithm>

void smMat44::setIdentity()
{
  set(  1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1 );
}

void smMat44::set( 
  float m00, float m01, float m02, float m03,
  float m10, float m11, float m12, float m13,
  float m20, float m21, float m22, float m23,
  float m30, float m31, float m32, float m33 )
{
  L0[0] = m00; L0[1] = m01; L0[2] = m02; L0[3] = m03;
  L1[0] = m10; L1[1] = m11; L1[2] = m12; L1[3] = m13;
  L2[0] = m20; L2[1] = m21; L2[2] = m22; L2[3] = m23;
  L3[0] = m30; L3[1] = m31; L3[2] = m32; L3[3] = m33;
}

void smMat44::set( float* src )
{
  float* dst = (float*)this;
  for( int i=0; i<16; i++ )
    *dst++ = *src++;    
}

void smMat44::transpose()
{
  std::swap( L1[ 0 ], L0[ 1 ] );
  std::swap( L2[ 0 ], L0[ 2 ] );
  std::swap( L3[ 0 ], L0[ 3 ] );
  std::swap( L2[ 1 ], L1[ 2 ] );
  std::swap( L3[ 1 ], L1[ 3 ] );
  std::swap( L3[ 2 ], L2[ 3 ] );
}

void smMul( const smMat44& a, const smMat44& b, smMat44& r )
{  
  smVec4 al0, al1, al2, al3;
  al0 = a.L0;
  al1 = a.L1;
  al2 = a.L2;
  al3 = a.L3;
  smVec4 bc0, bc1, bc2, bc3;
  b.getColumn<0>( bc0 );
  b.getColumn<1>( bc1 );
  b.getColumn<2>( bc2 );
  b.getColumn<3>( bc3 );
  r.L0[0] = smDot( al0, bc0 ); r.L0[1] = smDot( al0, bc1 ); r.L0[2] = smDot( al0, bc2 ); r.L0[3] = smDot( al0, bc3 );
  r.L1[0] = smDot( al1, bc0 ); r.L1[1] = smDot( al1, bc1 ); r.L1[2] = smDot( al1, bc2 ); r.L1[3] = smDot( al1, bc3 );
  r.L2[0] = smDot( al2, bc0 ); r.L2[1] = smDot( al2, bc1 ); r.L2[2] = smDot( al2, bc2 ); r.L2[3] = smDot( al2, bc3 );
  r.L3[0] = smDot( al3, bc0 ); r.L3[1] = smDot( al3, bc1 ); r.L3[2] = smDot( al3, bc2 ); r.L3[3] = smDot( al3, bc3 );
}

void smMul3( const smMat44& a, const smMat44& b, smMat44& r )
{  
  smVec4 al0, al1, al2, al3;
  al0 = a.L0;
  al1 = a.L1;
  al2 = a.L2;  
  smVec4 bc0, bc1, bc2;
  b.getColumn3<0>( bc0 );
  b.getColumn3<1>( bc1 );
  b.getColumn3<2>( bc2 );  
  r.L0[0] = smDot3( al0, bc0 ); r.L0[1] = smDot3( al0, bc1 ); r.L0[2] = smDot3( al0, bc2 );
  r.L1[0] = smDot3( al1, bc0 ); r.L1[1] = smDot3( al1, bc1 ); r.L1[2] = smDot3( al1, bc2 );
  r.L2[0] = smDot3( al2, bc0 ); r.L2[1] = smDot3( al2, bc1 ); r.L2[2] = smDot3( al2, bc2 );
}

void smMul( const smMat44& a, const smVec4& b, smVec4& r )
{
  smVec4 c;
  c.X = smDot( a.L0, b );
  c.Y = smDot( a.L1, b );
  c.Z = smDot( a.L2, b );
  c.W = smDot( a.L3, b );
  r = c;
}

void smMul3( const smMat44& a, const smVec4& b, smVec4& r )
{
  smVec4 c;
  c.X = smDot3( a.L0, b );
  c.Y = smDot3( a.L1, b );
  c.Z = smDot3( a.L2, b );    
  r.X = c.X;
  r.Y = c.Y;
  r.Z = c.Z;
  r.W = b.W;
}

bool smInverse( const smMat44& a, smMat44& r )
{
  float fA0 = a.L0[0]*a.L1[1] - a.L0[1]*a.L1[0];
  float fA1 = a.L0[0]*a.L1[2] - a.L0[2]*a.L1[0];
  float fA2 = a.L0[0]*a.L1[3] - a.L0[3]*a.L1[0];
  float fA3 = a.L0[1]*a.L1[2] - a.L0[2]*a.L1[1];
  float fA4 = a.L0[1]*a.L1[3] - a.L0[3]*a.L1[1];
  float fA5 = a.L0[2]*a.L1[3] - a.L0[3]*a.L1[2];
  float fB0 = a.L2[0]*a.L3[1] - a.L2[1]*a.L3[0];
  float fB1 = a.L2[0]*a.L3[2] - a.L2[2]*a.L3[0];
  float fB2 = a.L2[0]*a.L3[3] - a.L2[3]*a.L3[0];
  float fB3 = a.L2[1]*a.L3[2] - a.L2[2]*a.L3[1];
  float fB4 = a.L2[1]*a.L3[3] - a.L2[3]*a.L3[1];
  float fB5 = a.L2[2]*a.L3[3] - a.L2[3]*a.L3[2];

  float fDet = fA0*fB5-fA1*fB4+fA2*fB3+fA3*fB2-fA4*fB1+fA5*fB0;
  if (fabs(fDet) <= 0.00001f )
  {
    //smASSERT(0);
    return false;
  }    

  float fInvDet = 1.0f/fDet;

  r.set( 
    fInvDet * ( + a.L1[1]*fB5 - a.L1[2]*fB4 + a.L1[3]*fB3 ),
    fInvDet * ( - a.L0[1]*fB5 + a.L0[2]*fB4 - a.L0[3]*fB3 ),
    fInvDet * ( + a.L3[1]*fA5 - a.L3[2]*fA4 + a.L3[3]*fA3 ),
    fInvDet * ( - a.L2[1]*fA5 + a.L2[2]*fA4 - a.L2[3]*fA3 ),

    fInvDet * ( - a.L1[0]*fB5 + a.L1[2]*fB2 - a.L1[3]*fB1 ),
    fInvDet * ( + a.L0[0]*fB5 - a.L0[2]*fB2 + a.L0[3]*fB1 ),
    fInvDet * ( - a.L3[0]*fA5 + a.L3[2]*fA2 - a.L3[3]*fA1 ),
    fInvDet * ( + a.L2[0]*fA5 - a.L2[2]*fA2 + a.L2[3]*fA1 ),

    fInvDet * ( + a.L1[0]*fB4 - a.L1[1]*fB2 + a.L1[3]*fB0 ),
    fInvDet * ( - a.L0[0]*fB4 + a.L0[1]*fB2 - a.L0[3]*fB0 ),
    fInvDet * ( + a.L3[0]*fA4 - a.L3[1]*fA2 + a.L3[3]*fA0 ),
    fInvDet * ( - a.L2[0]*fA4 + a.L2[1]*fA2 - a.L2[3]*fA0 ),

    fInvDet * ( - a.L1[0]*fB3 + a.L1[1]*fB1 - a.L1[2]*fB0 ),
    fInvDet * ( + a.L0[0]*fB3 - a.L0[1]*fB1 + a.L0[2]*fB0 ),
    fInvDet * ( - a.L3[0]*fA3 + a.L3[1]*fA1 - a.L3[2]*fA0 ),
    fInvDet * ( + a.L2[0]*fA3 - a.L2[1]*fA1 + a.L2[2]*fA0 ) 
    );

  return true;
}

bool smInverse3( const smMat44& a, smMat44& r )
{
  float fA0 = a.L1[1]*a.L2[2] - a.L2[1]*a.L1[2];
  float fA1 = a.L1[0]*a.L2[2] - a.L2[0]*a.L1[2];
  float fA2 = a.L1[0]*a.L2[1] - a.L2[0]*a.L1[1];  

  float fDet = a.L0[0] * fA0 - a.L0[1] * fA1 + a.L0[2] * fA2;
  if (fabs(fDet) <= 0.00001f )
  {
    smASSERT(0);
    return false;
  }

  float fInvDet = 1.0f/fDet;

  float fA3 = a.L0[1]*a.L2[2] - a.L2[1]*a.L0[2];
  float fA4 = a.L0[0]*a.L2[2] - a.L2[0]*a.L0[2];
  float fA5 = a.L0[0]*a.L2[1] - a.L2[0]*a.L0[1];  
  float fA6 = a.L0[1]*a.L1[2] - a.L1[1]*a.L0[2];
  float fA7 = a.L0[0]*a.L1[2] - a.L1[0]*a.L0[2];
  float fA8 = a.L0[0]*a.L1[1] - a.L1[0]*a.L0[1];  

  r.L0[0] = fInvDet * fA0;
  r.L0[1] = -fInvDet * fA3;
  r.L0[2] = fInvDet * fA6;
  r.L1[0] = -fInvDet * fA1;
  r.L1[1] = fInvDet * fA4;
  r.L1[2] = -fInvDet * fA7;
  r.L2[0] = fInvDet * fA2;
  r.L2[1] = -fInvDet * fA5;
  r.L2[2] = fInvDet * fA8;  

  return true;
}

void smSetTranslation( const smVec4& tr, smMat44& m )
{
  smSetTranslation( tr.X, tr.Y, tr.Z, m );
}

void smSetTranslation( float trX, float trY, float trZ, smMat44& m )
{ 
  m.L0[3] = trX;
  m.L1[3] = trY;
  m.L2[3] = trZ;
}

void smGetTranslation( const smMat44& m, smVec4& tr )
{  
  tr.X = m.L0[3];
  tr.Y = m.L1[3];
  tr.Z = m.L2[3];
}

void smGetRotIMatrix( float ang, smMat44& r )
{
  float c,s;
  smCosSin( ang, c, s );  
  r.setIdentity();
  r.L1[1] = c;  r.L1[2] = s;
  r.L2[1] = -s;  r.L2[2] = c;
}

void smGetRotJMatrix( float ang, smMat44& r )
{
  float c,s;
  smCosSin( ang, c, s );
  r.setIdentity();
  r.L0[0] = c;  r.L0[2] = -s;
  r.L2[0] = s;  r.L2[2] = c;
}

void smGetRotKMatrix( float ang, smMat44& r )
{  
  float c,s;
  smCosSin( ang, c, s );
  r.setIdentity();
  r.L0[0] = c;  r.L0[1] = s;
  r.L1[0] = -s;  r.L1[1] = c;
}

void smGetXFormTRSMatrix( const smVec4& tr, const smEulerAngles& rot, float sc, smMat44& r )
{  
  smGetXFormTRSMatrix( tr.X, tr.Y, tr.Z, rot, sc, sc, sc, r ); 
}

void smGetXFormTRSMatrix( float trX, float trY, float trZ, const smEulerAngles& rot, float sc, smMat44& r )
{
  smGetXFormTRSMatrix( trX, trY, trZ, rot, sc, sc, sc, r ); 
}

void smGetXFormTRSMatrix( float trX, float trY, float trZ, const smEulerAngles& rot, float scX, float scY, float scZ, smMat44& r )
{
  r.setIdentity();  
  r.L0[0] = scX;
  r.L1[1] = scY;
  r.L2[2] = scZ;

  smMat44 rotMatrix;
  rot.getRotationMatrix( rotMatrix );

  smMul( rotMatrix, r, r );

  smSetTranslation( trX, trY, trZ, r );
}

void smGetScalingMatrix( float sX, float sY, float sZ, smMat44& r )
{
  r.setIdentity();  
  r.L0[0] = sX;
  r.L1[1] = sY;
  r.L2[2] = sZ;
}
