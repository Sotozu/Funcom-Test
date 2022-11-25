#pragma once 

#include "sas_delegate.h"

// -------------------------------------------------------------------------
//  Event
// -------------------------------------------------------------------------

template< typename Sig > 
class sasEvent 
{
private: //! types
  typedef sasEvent<Sig> This;
  typedef typename sasSigTraits<Sig>::ReturnType ReturnType;
  enum    { Arity = sasSigTraits<Sig>::Arity };
  typedef typename sasSigTraits<Sig>::Param0 Param0;

public: //! api
  void insert( const Delegate< Sig >& d ) { _list.insert( d ); }  
  void remove( const Delegate< Sig >& d ) { _list.erase( d ); }
  void raise();
  void raise( Param0 a );

private: //! members
  typedef std::set< Delegate< Sig > > Delegates;
  Delegates _list;
};

template< typename Sig > 
void sasEvent<Sig>::raise()
{
  sasSTATIC_ASSERT( Arity == 0 );
  Delegates::iterator i     = _list.begin();
  Delegates::iterator last  = _list.end();
  while( i != last )
  {
    (*i).call();
    ++i;
  }
}

template< typename Sig > 
void sasEvent<Sig>::raise( typename sasEvent<Sig>::Param0 p0 )
{
  sasSTATIC_ASSERT( Arity == 1 );
  Delegates::iterator i     = _list.begin();
  Delegates::iterator last  = _list.end();
  while( i != last )
  {
    (*i).call( p0 );
    ++i;
  }
}
