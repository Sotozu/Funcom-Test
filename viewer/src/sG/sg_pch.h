#ifndef SG_PCH_H
#define SG_PCH_H


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <new>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <stdint.h>

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#define alloca _alloca
#else
#include <stdint.h>
#endif

typedef uint64_t sgRscId;

#include <sasMaths.h>
#include <sas_main.h>
#include <su_main.h>
#include <ss_main.h>

#define FINAL 0

#define DISABLE_AUDIO 1

#endif //SG_PCH_H