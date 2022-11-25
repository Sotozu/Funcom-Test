#pragma once 

// -------------------------------------------------------------------------
//  Misc
// -------------------------------------------------------------------------

struct sasNullType {};

// -------------------------------------------------------------------------
//  SigTraits
// -------------------------------------------------------------------------

template< typename T >
struct sasSigTraits;

template< typename R >
struct sasSigTraits< R(void) >
{
  typedef R           ReturnType;
  typedef sasNullType    Param0 ;
  enum { Arity = 0 };
};

template< typename R, typename P0 >
struct sasSigTraits< R(P0) >
{
  typedef R           ReturnType;
  typedef P0          Param0 ;
  enum { Arity = 1 };
};


// -------------------------------------------------------------------------
//  If
// -------------------------------------------------------------------------

template< bool C, typename S, typename F >
struct sasIf;

template< typename S, typename F >
struct sasIf< true, S, F >
{
  typedef S Result;    
};

template< typename S, typename F >
struct sasIf< false, S, F >
{
  typedef F Result;    
};
