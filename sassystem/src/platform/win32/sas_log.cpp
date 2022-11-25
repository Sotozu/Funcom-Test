#include "ss_pch.h"
#include "threading/sas_spinlock.h"

sasSpinLock loggerLock;

void sasDebugOutput(const char* szFormat, ...)
{
  sasScopedSpinLock lock( loggerLock );

  char szBuff[1024];
  va_list arg;
  va_start(arg, szFormat);
  _vsnprintf_s(szBuff, sizeof(szBuff), _TRUNCATE, szFormat, arg);
  va_end(arg);

  OutputDebugString(szBuff);

}