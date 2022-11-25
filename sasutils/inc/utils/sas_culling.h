#pragma once

class sasFrustum;


float sasDistFromPlane( const smVec4* __restrict plane, const smVec4* __restrict point );
bool  sasIntersects( const sasFrustum* __restrict frustum, const smVec4* __restrict boundingSphere );
bool  sasIntersetRaySphere( const smVec3& origin, const smVec3& direction, const smVec3& sphereCenter, const float sphereRadius, float& dist );
