#pragma once 

const size_t SAS_GEOMETRY_MAX_LODS = 4;

class sasMesh;

struct sasGeometryLOD 
{
  sasMesh*  mesh;
  float     distance;

  sasGeometryLOD() : mesh(0), distance(0.f) {}
};

class sasGeometry
{
  sasNO_COPY( sasGeometry );
  sasMEM_OP( sasGeometry );

public:
  sasGeometry( sasGeometryLOD* lods, size_t nLods  );
  ~sasGeometry();

public:
  sasMesh* lod( size_t idx ) const          { sasASSERT(idx<SAS_GEOMETRY_MAX_LODS); return _lods[ idx ].mesh; };
  float    lodDistance( size_t idx ) const  { sasASSERT(idx<SAS_GEOMETRY_MAX_LODS); return _lods[ idx ].distance; };
  uint32_t numLods() const                  { return _numLods; }

private:
  sasGeometryLOD  _lods[ SAS_GEOMETRY_MAX_LODS ];
  uint32_t        _numLods;
};
