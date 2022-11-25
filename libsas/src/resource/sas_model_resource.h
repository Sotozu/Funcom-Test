#pragma once 

#include "sas_resource.h"
#include "utils/sas_stringid.h"

class sasVertexBuffer;
class sasIndexBuffer;
class sasSubMesh;
class sasMesh;
class sasGeometry;
class sasStagingInstance;
class sasCamera;
class sasMaterial;
class sasSkeletonMapping;
class sasSkeletonHierarchy;
class sasSkeleton;
class sasAnimation;

struct sasModelResourceData 
{
    std::vector< sasVertexBuffer* >     vertexBuffers;
    std::vector< sasIndexBuffer* >      indexBuffers;    
    std::vector< sasSubMesh* >          subMeshes;
    std::vector< sasMesh* >             meshes;
    std::vector< sasSkeletonMapping* >  skeletonMappings;
    std::vector< sasSkeleton* >         skeletons;
    std::vector< sasGeometry* >         geometries;
    std::vector< sasStagingInstance* >  instances;    
    std::vector< sasCamera* >           cameras;    
    std::vector< sasMaterial* >         materials;
    std::vector< sasAnimation* >        compatibleAnimations;
    sasSkeletonHierarchy*               skeletonHierarchy;
};

class sasModelResource : public sasResource 
{
  sasNO_COPY( sasModelResource );
  sasMEM_OP( sasModelResource );

public:
    sasModelResource( const sasModelResourceData& data );
    ~sasModelResource();

    void setName( sasStringId name )
    {
      _name = name;
    }

    sasStringId name() const { return _name; }
    const sasModelResourceData& resourceData() const { return _data; }

private:
    sasModelResourceData _data;
    sasStringId _name;
};