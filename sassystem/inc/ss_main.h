#pragma once 

#include "ss_export.h"
#include "sys/sas_mem.h"
#include "sys/sas_scratchmem.h"
#include "sys/sas_stackalloc.h"

// ----------------------------------------------------------------------------
//  Main
// ----------------------------------------------------------------------------

ssAPI bool sasSystemsInitialize();
ssAPI void sasSystemsUpdate();
ssAPI void sasSystemsShutdown();


// ----------------------------------------------------------------------------
//  sasStringId lookup
// ----------------------------------------------------------------------------

ssAPI void ssRegisterStringId( const uint32_t hash, const char* string );
ssAPI const char* ssLookupStringId( const uint32_t hash );
