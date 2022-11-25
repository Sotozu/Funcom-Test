#pragma once

class sasResource;

class sasResourceLoader 
{
public:
    virtual ~sasResourceLoader() {}

public:
    virtual sasResource* load( const char* path ) = 0;
    virtual sasResource* load( const char* path, const char* name ){ return load( path ); }
    virtual sasResource* load( const char* path, const char* name, const smVec3& scale, const smVec3& translation ){ return load( path ); }
};
