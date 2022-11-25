#include "sm_aabb.h"

smAABB::smAABB()
  : _min( 9999999.f )
  , _max( -9999999.f )    
{
  _min.W = _max.W = 1.f;
}

smAABB::smAABB( const smVec4& minCorner, const smVec4& maxCorner )
  : _min( minCorner )
  , _max( maxCorner )
{
  _min.W = _max.W = 1.f;
  smMax( _min, _max, _max );
}

void smAABB::unionWith( const smAABB& other )
{
  smMin3( _min, other._min, _min );
  smMax3( _max, other._max, _max );  
}

smVec4 smAABB::computeBoundingSphere() const
{
  smVec4 center = ( _min + _max ) * 0.5f;
  float rad = smLength3( _max - center );
  return smVec4( center.X, center.Y, center.Z, rad );
}

smVec3 smAABB::centrePoint() const
{
  return ( ( _min + _max ) * 0.5f );
}
