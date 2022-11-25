#pragma once 

#include "su_export.h"

/*suAPI*/ const char* sasPathGetExt( const char* path );
/*suAPI*/ void        sasPathGetFilename( const char* path, char* filename, int max_string_len );
/*suAPI*/ bool        sasPathGetRelativePathFromRscFolder( const char* path, char* relPath, int max_string_len );
/*suAPI*/ void        sasPathRemoveExtension( const char* path, char* dest, int max_string_len );
/*suAPI*/ bool        sasPathDoesFolderExist( const char* path );
/*suAPI*/ bool        sasPathDoesFileExist( const char* path );
