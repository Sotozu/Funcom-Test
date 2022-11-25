#include "sas_pch.h"
#include "sas_visibility.h"
#include "utils/sas_frustum.h"
#include "utils/sas_culling.h"
#include "render/sas_instance.h"
#include "render/debug/sas_debugmng.h"
#include "utils/sas_timer.h"
#include "sas_drawlist.h"


sasVisibility::sasVisibility()
{
  _queries.reserve(16);
}

sasVisibility::~sasVisibility()
{

}

sasVisibility::sasVisibilityQueryId  sasVisibility::StartVisibilityQuery(sasCullType cullType, unsigned int filter, const sasFrustum* __restrict pFrustum, sasDrawList* __restrict pDrawList)
{
  sasASSERT(pDrawList);
  sasASSERT(pFrustum);

  sasInstance::Iterator instanceIt  =   sasInstance::begin();
  sasInstance::Iterator last        =   sasInstance::end();
  unsigned int index = 0;

  unsigned int speedIndex = 0;
  if( cullType == e_CullType_Frustum )
  {
    while( instanceIt != last )
    {
      if( sasIntersects(pFrustum, (*instanceIt)->wsBoundingSphere() ) )
      {
        pDrawList->_drawList[index] = *instanceIt;
        ++index;
      }
    
      ++instanceIt;
    }
  }
  else 
  {
    sasASSERT( cullType == e_CullType_None ); //Check for unsupported culling types
    while( instanceIt != last )
    {
      pDrawList->_drawList[index] = *instanceIt;
      ++index;
      ++instanceIt;
    }
  }

  pDrawList->_size = index;

  VisibilityQueryData queryData;
  queryData._drawList = pDrawList;
  _queries.push_back( queryData );

  return _queries.size() - 1;
}

sasDrawList* sasVisibility::GetQueryResults(sasVisibilityQueryId id)
{
  sasASSERT( id < _queries.size() );
  sasDrawList* pDrawList = _queries[id]._drawList;
  if( id != ( _queries.size() - 1 ) )
  {
    _queries[id] = _queries[ _queries.size() - 1 ];
  }
  _queries.pop_back();

  return pDrawList;
}

