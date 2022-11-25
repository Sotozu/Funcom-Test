#include "ss_pch.h"
#include "threading/sas_atomic.h"

long sasAtomicCmpAndExchange( sasAtomic* atomic, long exchangeVal, long compareVal )
{
  return InterlockedCompareExchange( atomic, exchangeVal, compareVal );
}

long sasAtomicInc( sasAtomic* atomic )
{
  return InterlockedIncrement( atomic );
}

long sasAtomicDec( sasAtomic* atomic )
{
  return InterlockedDecrement( atomic );
}

long sasAtomicExchange( sasAtomic* atomic, long exchangeVal )
{
  return InterlockedExchange( atomic, exchangeVal );
}
