#include "sas_pch.h"

void sasDebugOutput(const char* szFormat, ...)
{
  char szBuff[1024];
  va_list arg;
  va_start(arg, szFormat);
  vsprintf(szBuff, szFormat, arg);
  va_end(arg);

  printf("sas: %s", szBuff);
}