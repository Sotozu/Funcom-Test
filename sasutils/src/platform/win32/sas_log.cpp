#include "su_pch.h"
#include "utils/sas_file.h"
#include "threading/sas_spinlock.h"
#define SAS_LOGTOFILE 1

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

#if SAS_LOGTOFILE
  static bool hasBeenInitialized = false;
  #define SAS_LOGFILENAME "sasLog.txt"
  sasFile logFile;
  if( !logFile.open( SAS_LOGFILENAME, hasBeenInitialized ? sasFile::AppendText : sasFile::WriteText ) )
  {
    MessageBox( 0, "Failed to create log file", "sasLogger", MB_OK|MB_ICONERROR );
  }
  logFile.write( reinterpret_cast< uint8_t* >( &szBuff[ 0 ] ), strlen( szBuff ) );
  hasBeenInitialized = true;
#endif
}