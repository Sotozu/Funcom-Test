#include "su_pch.h"
#include "utils/sas_path.h"
#include <string.h>

const char* sasPathGetExt( const char* path )
{
  sasASSERT( path );

  int s = strlen( path );
  
  while ( s && path[s-1]!='.' )
    --s;
  
  if( !s )
    return 0;

  return path+s;
}

void sasPathGetFilename( const char* path, char* filename, int max_string_len )
{
  sasASSERT( path );

  int s = strlen( path );

  //remove extension
  while ( s && path[s-1]!='.' )
    --s;

  //remove dot
  --s;

  if( !s )
    return;

  int end_char = s;

  while ( s && ( ( path[s-1]!='/' ) && ( path[s-1]!='\\' ) ) )
    --s;

  int i = 0;
  for( i = 0; i < ( end_char - s ); i++ )
  {
    if( i >= ( max_string_len - 1 ) )
    {
      break;
    }
    filename[ i ] = path[ s + i ];
  }

  filename[ i ] = 0;
}

void sasPathRemoveExtension( const char* path, char* dest, int max_string_len )
{
  sasASSERT( path );

  int s = strlen( path );

  //remove extension
  while ( s && path[s-1]!='.' )
    --s;

  //remove dot
  --s;

  if( !s )
    return;

  int end_char = s;
  int i;
  for( i = 0; i < end_char; i++ )
  {
    if( i >= ( max_string_len - 1 ) )
    {
      break;
    }
    dest[ i ] = path[ i ];
  }

  dest[ i ] = 0;
}

bool sasPathGetRelativePathFromRscFolder( const char* path, char* relPath, int max_string_len )
{
  sasASSERT( path );

  int pathLength = strlen( path );
  int s = pathLength;

  //remove extension
  while ( s && path[s-1]!='.' )
    --s;

  //remove dot
  --s;

  if( !s )
    return false;

  while( s )
  {
    //find previous folder section
    while ( s && ( ( path[s-1]!='/' ) && ( path[s-1]!='\\' ) ) )
      --s;

    //remove / or \\ 
    --s;
    if( !( s - 2 ) )
      return false;

    //check if correct folder
    if( path[ s - 3 ] == 'r' && path[ s - 2 ] == 's' && path[ s - 1 ] == 'c' )
    {
      s++; //move cursor after the \\ or /

      int i = 0;
      for( i = 0; i < ( pathLength - s ); i++ )
      {
        if( i >= ( max_string_len - 1 ) )
        {
          break;
        }

        relPath[ i ] = path[ s + i ];
      }

      relPath[ i ] = 0;

      return true;
    }
  }
  
  return false;
}


bool sasPathDoesFolderExist( const char* path )
{
  DWORD ftyp = GetFileAttributesA( path );
  if (ftyp == INVALID_FILE_ATTRIBUTES)
    return false;  

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
    return true;  

  return false;
}

bool sasPathDoesFileExist( const char* path )
{
  DWORD ftyp = GetFileAttributesA( path );
  if (ftyp == INVALID_FILE_ATTRIBUTES)
    return false;  

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
    return false;  

  return true;
}
