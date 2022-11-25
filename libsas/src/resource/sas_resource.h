#pragma once 

#include "sas_export.h"
#include "utils/sas_refcount.h"

class sasResource : public sasRefCounter
{
public:
    virtual ~sasResource() {}
};

