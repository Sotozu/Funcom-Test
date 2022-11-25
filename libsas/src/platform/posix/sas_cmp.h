#pragma once 

#include "utils/sas_singleton.h"
#include <vector>

/* 
    Contiguous memory pool
    __________________________________________________________________________________________________
    This should only be used for class member variables which will be used for batch processing. 
    Access to individual variables will have an additional level of indirection, 
    destruction will be slower and memory consumption will be increased by the size of a pointer.
*/

#define _CMPVAR_MEMBERS(TYPE, NAME, MAX_POOL_SIZE) \
    static int _getIdFor##NAME() { static int id = sasContiguousMemPool::getNextId(); return id; } \
    sasContiguousMemPoolVar<TYPE, _getIdFor##NAME, MAX_POOL_SIZE> NAME;

#define _CMPVAR_ACCESSOR(TYPE, NAME) \
    static TYPE * getArray##NAME() { return reinterpret_cast<TYPE*>(sasContiguousMemPool::getArray( _getIdFor##NAME() )); }

//Macros to be used to create member variables from CMP
#define CMPVAR_PUBLIC(TYPE, NAME, MAX_POOL_SIZE) \
   _CMPVAR_MEMBERS(TYPE, NAME, MAX_POOL_SIZE) \
   _CMPVAR_ACCESSOR(TYPE, NAME)

#define CMPVAR_PRIVATE(TYPE, NAME, MAX_POOL_SIZE) \
  _CMPVAR_MEMBERS(TYPE, NAME, MAX_POOL_SIZE) \
  public: \
  _CMPVAR_ACCESSOR(TYPE, NAME) \
  private:

//Internals, never directly used by external code
// VD: comment not needed. This header is not part of the interface.
class sasContiguousMemPool : public sasSingleton< sasContiguousMemPool >
{
public:
  sasContiguousMemPool() { _memDataArray.reserve(64); }
  ~sasContiguousMemPool();

  template<typename T>
  static void getNextPtr(int id, unsigned int maxPoolSize, T** ppVAr)
  {
    int idIndex = findIndex(id);

    if(idIndex == cmpInvalidIndex)
    {
      sasContiguousMemPool::memData md;
      md.id = id;
      md.currentPos = 0;
      md.maxSize = maxPoolSize;
      md.pData = sasMalloc(md.maxSize * sizeof(T));
      md.pPtrArray = sasMalloc(md.maxSize * sizeof(T**));
      idIndex = _memDataArray.size();
      _memDataArray.push_back(md);
    }

    sasASSERT(_memDataArray[idIndex].currentPos < (_memDataArray[idIndex].maxSize - 1));
    T* memArray = reinterpret_cast<T*>(_memDataArray[idIndex].pData);
    unsigned int elementIndex = _memDataArray[idIndex].currentPos;
    _memDataArray[idIndex].currentPos++;
    T*** ptrValue = reinterpret_cast<T***>(reinterpret_cast<char*>(_memDataArray[idIndex].pPtrArray) + (sizeof(T**) * elementIndex));
    *ptrValue = ppVAr;
    *ppVAr = &memArray[elementIndex];
  }

  static int getNextId()
  {
    static int idCounter = 0;
    return idCounter++;
  }

  static void* getArray(int id)
  {
    int idIndex = findIndex(id);
    if(idIndex == cmpInvalidIndex)
      return NULL;

    return _memDataArray[idIndex].pData;
  }

  template<typename T>
  static void removeFromPool(int id, T* var)
  {
    int idIndex = findIndex(id);
    sasASSERT(idIndex != cmpInvalidIndex);
    T* memArray = reinterpret_cast<T*>(_memDataArray[idIndex].pData);
    intptr_t arrayStart = reinterpret_cast<intptr_t>(memArray);
    intptr_t varPos = reinterpret_cast<intptr_t>(var);
    unsigned int indexInArray = (varPos - arrayStart) / sizeof(T);
    unsigned int lastIndex = _memDataArray[idIndex].currentPos - 1;

    sasASSERT(indexInArray <= lastIndex);
    if(indexInArray < lastIndex) //swap element with last element
    {
      memArray[indexInArray] = memArray[lastIndex];
      T*** ptrValueDest = reinterpret_cast<T***>(reinterpret_cast<char*>(_memDataArray[idIndex].pPtrArray) + (sizeof(T**) * indexInArray));
      T*** ptrValueSource = reinterpret_cast<T***>(reinterpret_cast<char*>(_memDataArray[idIndex].pPtrArray) + (sizeof(T**) * lastIndex));
      ptrValueDest = ptrValueSource;
      //Update moved var ptr to new position in pool
      **ptrValueDest = &memArray[indexInArray];
    }
    _memDataArray[idIndex].currentPos--;
  }

private:
  static int findIndex(int id);

private:
  struct memData
  {
    int id;
    unsigned int currentPos;
    unsigned int maxSize;
    void* pData;      //actual data in pool
    void* pPtrArray;  //array of ptrs to the CMPVar objects pointing to the pool data
  };
  static std::vector<memData> _memDataArray;

  static const int cmpInvalidIndex;
};

template<typename T, int (*IDFUNC)(), unsigned int MAX_POOL_SIZE>
struct sasContiguousMemPoolVar
{
  T* var;

  sasContiguousMemPoolVar()
  {
    sasContiguousMemPool::getNextPtr<T>(IDFUNC(), MAX_POOL_SIZE, &var);
  }

  ~sasContiguousMemPoolVar()
  {
    sasContiguousMemPool::removeFromPool<T>(IDFUNC(), var);
  }

  T* getPtr() {return var;}
  T& get() {return *var;}
};



