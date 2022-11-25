#include "sas_pch.h"
#include "sas_cmp.h"

std::vector<sasContiguousMemPool::memData> sasContiguousMemPool::_memDataArray;

const int sasContiguousMemPool::cmpInvalidIndex = -1;

sasContiguousMemPool::~sasContiguousMemPool()
{
  for(unsigned int i = 0; i < _memDataArray.size(); i++)
  {
    sasASSERT(_memDataArray[i].currentPos == 0); //All data should already have been freed
    sasFree(_memDataArray[i].pData);
    sasFree(_memDataArray[i].pPtrArray);
  }
}



int sasContiguousMemPool::findIndex(int id)
{
  for (unsigned int i = 0; i < _memDataArray.size(); i++)
  {
    if(_memDataArray[i].id == id)
    {
      return i;
    }
  }

  return cmpInvalidIndex;
}

