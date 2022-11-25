project "sasUtils"
	uuid("34831360-bcdb-11e0-962b-0809668c9a43")
	kind "StaticLib"
	language "C++"  

	-----------------------------------------------------
	-- THIS
	-----------------------------------------------------			
	defines { "sasUtilsEXPORT" }
	includedirs { "inc/" }	
	includedirs { "src/" }		
	includedirs { "src/" }

	files { "inc/**.h", "src/**.cpp", "src/**.h" }

	configuration "macosx"
		excludes { "src/platform/win32/**" }
		excludes { "inc/platform/win32/**" }
		includedirs { "src/platform/posix/"}
		includedirs { "inc/platform/posix/"}
		pchheader "sasutils/src/su_pch.h"
	configuration "windows"
		excludes { "src/platform/posix/**" }
		excludes { "inc/platform/posix/**" }
		includedirs { "src/platform/win32/"}
		includedirs { "inc/platform/win32/"}
		pchheader "su_pch.h"
		pchsource "src/su_pch.cpp"
	configuration {}
	
	configuration "Debug"
		defines { "sasDEBUG" }
	configuration {}
	
	-----------------------------------------------------
	-- EXPORT	
	-----------------------------------------------------		
	sasutils_INCDIR = path.getabsolute("inc")
	
	configuration "macosx"
		sasutils_PLATFORM_INCDIR = path.getabsolute("inc/platform/posix/")
	configuration "windows"
		sasutils_PLATFORM_INCDIR = path.getabsolute("inc/platform/win32/")
	configuration {}
	
	-----------------------------------------------------
	-- sasMaths
	-----------------------------------------------------
	
	includedirs { sasmaths_INCDIR }
	links 		{ "sasMaths" }
	
	-----------------------------------------------------
	-- sasSystem
	-----------------------------------------------------
	
	includedirs { sassystem_INCDIR }
	includedirs { sassystem_PLATFORM_INCDIR }
	links 		{ "sasSystem" }
	
	-----------------------------------------------------
	-- JANSSON
	-----------------------------------------------------
	
	includedirs { jansson_INCDIR }	
	libdirs		{ jansson_LIBDIR }

	configuration "Debug"
		links 		{ "jansson_d" }		
	configuration "Release"
		links 		{ "jansson" }
	configuration {}
	