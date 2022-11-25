#pragma once

#include "ss_export.h"

typedef volatile long sasAtomic;

ssAPI long sasAtomicCmpAndExchange( sasAtomic* atomic, long exchangeVal, long compareVal );
ssAPI long sasAtomicInc( sasAtomic* atomic );
ssAPI long sasAtomicDec( sasAtomic* atomic );
ssAPI long sasAtomicExchange( sasAtomic* atomic, long exchangeVal );
