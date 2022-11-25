#include "sas_pch.h"
#include "sas_geometry.h"
#include "sasMaths.h"

sasGeometry::sasGeometry( sasGeometryLOD* lods, size_t nLods  )
{
  sasASSERT( nLods <= SAS_GEOMETRY_MAX_LODS );
  nLods = smMin( nLods, SAS_GEOMETRY_MAX_LODS );
  _numLods = nLods;
  for( size_t i=0; i<nLods; i++ )
  {
    sasASSERT( lods->mesh );
    _lods[i].mesh     = lods->mesh;
    _lods[i].distance = lods->distance;
  }
}

sasGeometry::~sasGeometry()
{
}
