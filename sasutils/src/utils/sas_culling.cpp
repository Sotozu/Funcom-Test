#include "su_pch.h"
#include "utils/sas_culling.h"
#include "utils/sas_frustum.h"


float sasDistFromPlane(const smVec4* __restrict plane, const smVec4* __restrict point)
{
  sasASSERT(plane);
  sasASSERT(point);
  return smDot3(*plane, *point) + plane->W;
}

bool sasIntersects(const sasFrustum* __restrict frustum, const smVec4* __restrict boundingSphere)
{
  sasASSERT(frustum);
  sasASSERT(boundingSphere);
  bool result = true;
  for ( int i = 0; i < 6; i++ )
  {
    result &= ( sasDistFromPlane(&(frustum->_planes[i]), boundingSphere) + boundingSphere->W >= 0 );
  }
  return result;
}

bool sasIntersetRaySphere( const smVec3& origin, const smVec3& direction, const smVec3& sphereCenter, const float sphereRadius, float& dist )
{
  float a = smDot3( direction, direction );
  float b = 2.f * smDot3( direction, ( origin - sphereCenter ) ); 
  float c = smDot3( sphereCenter, sphereCenter ) + smDot3( origin, origin ) - 2.f * smDot3( origin, sphereCenter ) - ( sphereRadius * sphereRadius );
  float disc = b * b - 4 * a * c;

  if( disc < 0.f )
    return false;

  float distSqrt = sqrtf( disc );
  float q;
  if( b < 0 )
    q = ( -b - distSqrt ) / 2.f;
  else
    q = ( -b + distSqrt ) / 2.f;

  float t0 = q / a;
  float t1 = c / q;

  if( t0 > t1 )
  {
    float temp = t0;
    t0 = t1;
    t1 = temp;
  }

  if( t1 < 0.f )
    return false;

  if( t0 < 0.f )
  {
    dist = t1;
    return true;
  }
  else
  {
    dist = t0;
    return true;
  }
}