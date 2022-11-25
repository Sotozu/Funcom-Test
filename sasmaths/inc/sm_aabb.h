#pragma once 

#include "sm_common.h"
#include "sm_vector.h"

smALIGN16_PRE class smAABB
{
public:
  smAABB();
  smAABB( const smVec4& minCorner, const smVec4& maxCorner );

public:  
  void    unionWith( const smAABB& other );
  smVec4  computeBoundingSphere() const;
  smVec3  centrePoint() const;

  smVec4 _min;
  smVec4 _max;
} smALIGN16_POST; 