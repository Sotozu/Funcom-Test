#pragma once 

#include "sas_meta.h"

// -------------------------------------------------------------------------
//  Delegate
// -------------------------------------------------------------------------

template< typename Sig >
class Delegate 
{
private: //! types
  
  enum    { Arity = sasSigTraits<Sig>::Arity };

  typedef Delegate<Sig> This;
  typedef typename sasSigTraits<Sig>::ReturnType ReturnType;  
  typedef typename sasSigTraits<Sig>::Param0 Param0;
  typedef ReturnType (*Stub) ( void* obj, const char* data, Param0 );

private: //! static methods

  static ReturnType stubFunction0( void* object, const char* data, sasNullType );
  static ReturnType stubFunction1( void* object, const char* data, Param0 );

  template< typename T > static ReturnType stubMethod0( void* object, const char* data, sasNullType );
  template< typename T > static ReturnType stubMethod1( void* object, const char* data, Param0 ); 

public: //! ctor/dtor

  Delegate() : _stub(0), _object(0) { memset(_fnData,0,sizeof(_fnData) ); }

  Delegate( ReturnType (*function) () ) 
  {    
    _stub = (Stub)stubFunction0;    
    _object = 0;
    memcpy( _fnData, &function, sizeof(function) );
  };  

  Delegate( ReturnType (*function) ( Param0 ) ) 
  {    
    _stub = (Stub)stubFunction1;    
    _object = 0;
    memcpy( _fnData, &function, sizeof(function) );
  };  

  template< typename T >
  Delegate( T* ptr, ReturnType (T::* method )() )
  {
    _stub = (Stub)&This::stubMethod0<T>;    
    _object = ptr;
    memcpy( _fnData, &method, sizeof(method) );
  }

  template< typename T >
  Delegate( T* ptr, ReturnType (T::* method )( Param0 ) )
  { 
    _stub = (Stub) &This::stubMethod1<T>;
    _object = ptr;
    memcpy( _fnData, &method, sizeof(method) );
  }

public: //! api

  void call() const
  {
    sasSTATIC_ASSERT( Arity == 0 );    
    ((Stub)_stub)( _object, _fnData, sasNullType() );
  }

  void call( Param0 p0 ) const
  {
    sasSTATIC_ASSERT( Arity == 1 );    
    ((Stub)_stub)( _object, _fnData, p0 );
  }

public: //! operators
  bool operator<( const Delegate<Sig>& rhs ) const {
    if( _object < rhs._object ) return true;
    return false;
  }

private:  //! members
  Stub            _stub;
  void*           _object;
  char            _fnData[16];
};

template< typename Sig >
inline typename Delegate<Sig>::ReturnType Delegate<Sig>::stubFunction0( void* object, const char* data, sasNullType )
{
  typedef Delegate<Sig>::ReturnType (* FunctionPtr )();
  FunctionPtr* fn = (FunctionPtr*)(void*)data;
  return (*fn)();
}

template< typename Sig >
inline typename Delegate<Sig>::ReturnType Delegate<Sig>::stubFunction1( void* object, const char* data, typename Delegate<Sig>::Param0 p0 )
{
  typedef Delegate<Sig>::ReturnType (* FunctionPtr )( Delegate<Sig>::Param0 );
  FunctionPtr* fn = (FunctionPtr*)(void*)data;
  return (*fn)( p0 );
}

template< typename Sig >
template< typename T >
inline typename Delegate<Sig>::ReturnType Delegate<Sig>::stubMethod0( void* object, const char* data, sasNullType )
{
  typedef Delegate<Sig>::ReturnType (T::* MemberPtr )();
  T* ptr = (T*)object;
  MemberPtr fn = *(MemberPtr*)(void*)data;  
  return (ptr->*fn)();
}

template< typename Sig >
template< typename T >
inline typename Delegate<Sig>::ReturnType Delegate<Sig>::stubMethod1( void* object, const char* data, typename Delegate<Sig>::Param0 p0 )
{
  typedef Delegate<Sig>::ReturnType (T::* MemberPtr )( typename Delegate<Sig>::Param0 );
  T* ptr = (T*)object;
  MemberPtr fn = *(MemberPtr*)(void*)data;  
  return (ptr->*fn)( p0 );
}

