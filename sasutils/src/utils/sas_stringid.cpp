#include "su_pch.h"
#include "utils/sas_stringid.h"
#include "ss_main.h"

sasStringId sasStringId::invalid()
{
  static sasStringId sInvalidId;
  return sInvalidId;
}

sasStringId sasStringId::build( const char* str )
{
  sasStringId result;

  Key hash = 5381;
  size_t len = strlen( str );
  int c;
  for ( size_t i = 0; i < len; i++ )
  {
    c = str[i];
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }	
  result._hash = hash;
  result.setDebugString( str );

  #ifndef FINAL
    //todo: made thread safe eventually
    ssRegisterStringId( hash, str );
  #endif

  return result;
}

const char* sasStringId::string() const
{
  #ifndef FINAL
    return ssLookupStringId( _hash );
  #else
    return NULL;
  #endif
}
