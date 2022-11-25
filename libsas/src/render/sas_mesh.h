#pragma once 

class sasVertexBuffer;
class sasIndexBuffer;
class sasMaterial;

class sasSubMesh;

sasALIGN16_PRE class sasMesh 
{
  sasNO_COPY( sasMesh );
  sasMEM_OP( sasMesh );

public:
  sasMesh( sasSubMesh** subMeshes, size_t nSubMeshes );
  ~sasMesh();

public:
  size_t subMeshCount() const { return _nSubMeshes; }
  sasSubMesh* subMesh( size_t i ) const { sasASSERT( i < _nSubMeshes ); return _submeshes[i]; }
  const smVec4* boundingSphere() const { return &_boundingSphere; }
  void setBoundingSphere( const smVec4& bs ) { _boundingSphere = bs; }

private:
    sasSubMesh** _submeshes;
    size_t _nSubMeshes;
    smVec4 _boundingSphere;
} sasALIGN16_POST;

struct sasSubMeshSetup 
{
  sasVertexBuffer*  vb;
  sasIndexBuffer*   ib;
  size_t            firstIndex;
  size_t            nPrims;
  smVec4            boundingSphere;
  sasMaterial*      material;
  bool              isSkinned;
};

sasALIGN16_PRE class sasSubMesh 
{
  sasNO_COPY( sasSubMesh );
  sasMEM_OP( sasSubMesh );

public:
  sasSubMesh( const sasSubMeshSetup& setup );
  ~sasSubMesh();

public:
  sasVertexBuffer*  vertexBuffer() const { return _vertexBuffer; }
  sasIndexBuffer*   indexBuffer() const { return _indexBuffer; }  
  size_t            primitiveCount() const { return _nPrimitives; }
  const smVec4&     boundingSphere() const { return _boundingSphere; }
  sasMaterial*      material() const    { return _material; }
  bool              isSkinned() const   { return _isSkinned; }
  size_t            firstIndex() const  { return _firstIndex; }
  size_t            numPrims() const    { return _nPrimitives; }

private:
  sasVertexBuffer* const  _vertexBuffer;
  sasIndexBuffer* const   _indexBuffer;
  const size_t            _firstIndex;
  const size_t            _nPrimitives;
  smVec4                  _boundingSphere;
  sasMaterial*            _material;
  bool                    _isSkinned;
} sasALIGN16_POST;
