#include "sm_vector.h"
#include <pmmintrin.h>

float smDot( const smVec4& v1, const smVec4& v2 )
{
  __m128 tmp = _mm_mul_ps(v1.R128, v2.R128);
  __m128 tmp2 = _mm_add_ps(tmp, smSWIZZLE_YXWZ(tmp));
  tmp = _mm_add_ss(tmp2, smSWIZZLE_ZWXY(tmp2));
  return _mm_cvtss_f32(tmp);
}

//NEW - Alex Soto
//Made this SIMD
float smDot3( const smVec4& v1, const smVec4& v2 )
{
  __m128 a = _mm_set_ps(v1[0], v1[1], v1[2], 0);
  __m128 b = _mm_set_ps(v2[0], v2[1], v2[2], 0);
  __m128 tmp = _mm_mul_ps(a, b);
  tmp = _mm_hadd_ps(tmp, tmp);
  tmp = _mm_hadd_ps(tmp, tmp);

  return _mm_cvtss_f32(tmp);
}

smVec4 smCross( const smVec4& v1, const smVec4& v2 )
{
  __m128 tmp = _mm_mul_ps(smSWIZZLE_YZXW(v1.R128), smSWIZZLE_ZXYW(v2.R128));
  __m128 tmp2 = _mm_mul_ps(smSWIZZLE_ZXYW(v1.R128), smSWIZZLE_YZXW(v2.R128));
  return _mm_sub_ps(tmp, tmp2);
}

void smCross( const smVec4& v1, const smVec4& v2, smVec4& r )
{
  r.X = v1.Y * v2.Z - v1.Z * v2.Y;
  r.Y = v1.Z * v2.X - v1.X * v2.Z;
  r.Z = v1.X * v2.Y - v1.Y * v2.X;
}
