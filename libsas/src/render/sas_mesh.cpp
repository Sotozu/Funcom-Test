#include "sas_pch.h"
#include "sas_mesh.h"

void getBoundingSphere(const smVec4& bs1, const smVec4& bs2, smVec4& r)
{  
  smVec3 diagonal( 1.f, 1.f, 1.f );
  smNormalize3( diagonal, diagonal );
  smVec3 min1 = bs1 - ( diagonal * bs1.W );
  smVec3 max1 = bs1 + ( diagonal * bs1.W );
  smVec3 min2 = bs2 - ( diagonal * bs2.W );
  smVec3 max2 = bs2 + ( diagonal * bs2.W );
  smVec3 min;
  smMin3( min1, min2, min );
  smVec3 max;
  smMax3( max1, max2, max );
  smVec3 centrePoint = (min + max) / 2.f;
  float radius = smLength3( max - centrePoint );
  r.set( centrePoint.X, centrePoint.Y, centrePoint.Z, radius );
}

sasMesh::sasMesh( sasSubMesh** subMeshes, size_t nSubMeshes )
  : _submeshes(0),
  _nSubMeshes(nSubMeshes)
{
  sasASSERT( _nSubMeshes );
  _submeshes = (sasSubMesh**)sasMalloc( _nSubMeshes * sizeof(sasSubMesh*) );

  _boundingSphere = subMeshes[0]->boundingSphere();
  for( size_t i=0; i<_nSubMeshes; i++ )
  {
    sasASSERT( subMeshes[i] );
    _submeshes[i] = subMeshes[i];
    getBoundingSphere( _boundingSphere, subMeshes[i]->boundingSphere(), _boundingSphere );
  }
}

sasMesh::~sasMesh()
{
  sasFree( _submeshes );
}

sasSubMesh::sasSubMesh( const sasSubMeshSetup& setup )
  : _vertexBuffer( setup.vb )
  , _indexBuffer( setup.ib )
  , _firstIndex( setup.firstIndex )
  , _nPrimitives( setup.nPrims )  
  , _boundingSphere( setup.boundingSphere )
  , _material( setup.material )
  , _isSkinned( setup.isSkinned )
{
}

sasSubMesh::~sasSubMesh()
{

}
