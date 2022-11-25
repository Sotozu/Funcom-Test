#include "su_pch.h"

#include "utils/sas_reflection.h"

sasReflectionTypeDB::sasReflectionTypeDB()
{
  _types.reserve(kMaxReflectionTypes);
  
  GetType<float>().IsBuiltin(true);
  GetType<double>().IsBuiltin(true);
  GetType<int>().IsBuiltin(true);
  GetType<unsigned int>().IsBuiltin(true);
  GetType<bool>().IsBuiltin(true);
}

void sasReflectionTypeDB::PrintObject(sasReflectionType& type, void* data)
{
  printf("Object: \n");
  std::vector<sasReflectionField>::iterator iElement = type._fields.begin();
  std::vector<sasReflectionField>::iterator iEnd = type._fields.end();
  for( ; iElement != iEnd; ++iElement )
  {
    
  }
}
