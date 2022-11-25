#pragma once 
//#define SAS_STACK_ALLOCS_DEBUG_USING_HEAP_LEAKING
#ifndef SAS_STACK_ALLOCS_DEBUG_USING_HEAP_LEAKING
  #define sasSTACK_ALLOC( TYPE, N_ITEMS )   (TYPE*)sasALIGNON16( alloca( N_ITEMS * sizeof(TYPE) + 16 ) )
#else
  #define sasSTACK_ALLOC( TYPE, N_ITEMS )   (TYPE*)sasMalloc( N_ITEMS * sizeof(TYPE) )
#endif

