#pragma once

#include "utils/sas_singleton.h"

class sasInstance;
class sasFrustum;
class sasDrawList;

enum sasCullType
{
  e_CullType_Frustum = 0,
  e_CullType_FrustumThreaded,
  e_CullType_None,
};

class sasVisibility : public sasSingleton< sasVisibility >
{
  sasMEM_OP( sasCamera );
public:
  sasVisibility();
  ~sasVisibility();

  typedef unsigned int sasVisibilityQueryId;
  sasVisibilityQueryId  StartVisibilityQuery(sasCullType cullType, unsigned int filter, const sasFrustum* __restrict pFrustum, sasDrawList* __restrict pDrawList);
  sasDrawList*          GetQueryResults(sasVisibilityQueryId id);

private:
  struct VisibilityQueryData
  {
    sasDrawList* _drawList;
  };

  std::vector<VisibilityQueryData> _queries;
};
