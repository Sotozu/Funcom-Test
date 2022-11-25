#include "ss_pch.h"
#include "ss_main.h"
#include "ss_core.h"

#include "utils/sas_timer.h"


static sasSystemCore* s_coreSystem = 0;
static char     s_coreSystemBuffer[ sizeof( sasSystemCore ) ];


// ----------------------------------------------------------------------------
//  sasStringId lookup
// ----------------------------------------------------------------------------

typedef std::map< uint32_t, std::string > HashStringLookupMap;
static HashStringLookupMap gStringIdLookup;

void ssRegisterStringId( uint32_t hash, const char* string )
{
  std::string str = string;
  gStringIdLookup[ hash ] = str;
}

const char* ssLookupStringId( uint32_t hash )
{
  HashStringLookupMap::const_iterator it = gStringIdLookup.find( hash );
  if( it != gStringIdLookup.end() )
  {
    return it->second.c_str();
  }
  else
  {
    return NULL;
  }
}

// ----------------------------------------------------------------------------
//  Main
// ----------------------------------------------------------------------------

bool sasSystemsInitialize()
{
  if( !s_coreSystem )
  {
    s_coreSystem = new( s_coreSystemBuffer ) sasSystemCore( );
  }

  return s_coreSystem != 0;
}

void sasSystemsUpdate()
{
  sasASSERT( s_coreSystem );
  s_coreSystem->update();
}

void sasSystemsShutdown()
{
  sasASSERT( s_coreSystem );

  gStringIdLookup.clear();

  s_coreSystem->~sasSystemCore();
  s_coreSystem = 0;
}


