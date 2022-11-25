#pragma once

#include "sas_stringid.h"

template <typename TYPE>
sasStringId GetTypeId()
{
  sasASSERT(!"GetTypeId() for this type needs to be implemented.");
}
 
template <> inline sasStringId GetTypeId<float>() { static sasStringId id = sasStringId::build("float"); return id; }
template <> inline sasStringId GetTypeId<double>() { static sasStringId id = sasStringId::build("double"); return id; }
template <> inline sasStringId GetTypeId<int>() { static sasStringId id = sasStringId::build("int"); return id; }
template <> inline sasStringId GetTypeId<unsigned int>() { static sasStringId id = sasStringId::build("uint"); return id; }
template <> inline sasStringId GetTypeId<bool>() { static sasStringId id = sasStringId::build("bool"); return id; }
template <> inline sasStringId GetTypeId<class sasPointLight>() { static sasStringId id = sasStringId::build("sasPointLight"); return id; }
template <> inline sasStringId GetTypeId<struct smVec4>() { static sasStringId id = sasStringId::build("smVec4"); return id; }
    
struct sasReflectionField
{
  sasStringId typeId;
  const char* fieldName;
  sasStringId fieldId;
  bool isPointer;
  size_t offset;
};


struct sasReflectionType
{  
public:
  template <typename OBJECT_TYPE, typename FIELD_TYPE>
  void AddField(const char* fieldName, FIELD_TYPE OBJECT_TYPE::*field)
  {
    sasReflectionField newField;
    newField.fieldName = fieldName;
    newField.fieldId = sasStringId::build(fieldName);
    newField.typeId = GetTypeId< typename StripPointer<FIELD_TYPE>::Type >();
    newField.isPointer = IsPointer<FIELD_TYPE>::val;
    // Will fail badly if operator& is overloaded on the object.
    newField.offset = (size_t)&(((OBJECT_TYPE *)0)->*field);

    printf("Registered field %s with offset %u\n", fieldName, (unsigned int)newField.offset);
    _fields.push_back(newField);
  }

  void IsBuiltin(bool isBuiltin) { _isBuiltin = isBuiltin; }
  sasStringId _id;
  size_t _size;
  bool _isBuiltin;
  std::vector<sasReflectionField> _fields;

private:
  template <typename TYPE>
  struct IsPointer
  {
    static const bool val = false;
  };

  template <typename TYPE>
  struct IsPointer<TYPE*>
  {
    static const bool val = true;
  };

  template <typename TYPE>
  struct StripPointer
  {
    typedef TYPE Type;
  };

  template <typename TYPE>
  struct StripPointer<TYPE*>
  {
    typedef TYPE Type;
  };

};

struct IsSameTypePredicate
{
  IsSameTypePredicate(const sasStringId& type): stringId(type) {}
  bool operator()(const sasReflectionType& type) const { return type._id == stringId; }
private:
  sasStringId stringId;
};

class sasReflectionTypeDB
{
public:
  static sasReflectionTypeDB& Get() { static sasReflectionTypeDB instance; return instance; }
  
  sasReflectionTypeDB();
  
  template<typename T>
  sasReflectionType& GetType()
  {
    sasStringId typeId = GetTypeId<T>();
    std::vector<sasReflectionType>::iterator iType = std::find_if(_types.begin(), _types.end(), IsSameTypePredicate(typeId));

    if( iType != _types.end() )
    {
      return *iType;
    }

    // We never want this to grow run-time.
    sasASSERT( _types.size() < kMaxReflectionTypes );

    _types.resize( _types.size() + 1 );
    sasReflectionType& newType = _types.back();

    newType._id = typeId;
    newType._size = sizeof(T);
    newType._isBuiltin = false;
  
    return newType;
  }

  void PrintObject(sasReflectionType& type, void* data);

  template<typename T>
  void PrintObject(T& data)
  {
    sasStringId typeId = GetTypeId<T>();
    std::vector<sasReflectionType>::iterator iType = std::find_if(_types.begin(), _types.end(), IsSameTypePredicate(typeId));

    if( iType != _types.end() )
    {
      PrintObject(*iType, (void *)&data);
      return;
    }
    sasASSERT(!"Type not in type database.");
  }

private:
  static const size_t kMaxReflectionTypes = 64;

  std::vector<sasReflectionType> _types;
};

template<typename T>
class Reflection_GlobalInitializer
{
  public:
  Reflection_GlobalInitializer(const char* className)
  {
    printf("Registering: %s\n", className);
    T::RegisterReflectionFields( sasReflectionTypeDB::Get().GetType<T>() );
  }
};

#define REGISTER_REFLECTION_CLASS(CLASSNAME) \
  Reflection_GlobalInitializer<CLASSNAME> s_globalReflectionInitializer_##CLASSNAME(#CLASSNAME);

