project "sasSystem"
	uuid("34831360-bcdb-11e0-962b-6689668c9a43")
	kind "SharedLib"
	language "C++"  

	-----------------------------------------------------
	-- THIS
	-----------------------------------------------------			
	defines { "sasSystemsEXPORT" }
	includedirs { "inc/" }	
	includedirs { "src/" }		
	includedirs { "src/" }

	files { "inc/**.h", "src/**.cpp", "src/**.h" }

	configuration "macosx"
		excludes { "src/platform/win32/**" }
		excludes { "inc/platform/win32/**" }
		includedirs { "src/platform/posix/"}
		includedirs { "inc/platform/posix/"}
		pchheader "sassystem/src/ss_pch.h"
	configuration "windows"
		excludes { "src/platform/posix/**" }
		excludes { "inc/platform/posix/**" }
		includedirs { "src/platform/win32/"}
		includedirs { "inc/platform/win32/"}
		pchheader "ss_pch.h"
		pchsource "src/ss_pch.cpp"
	configuration {}
	
	configuration "Debug"
		defines { "sasDEBUG" }
	configuration {}
	
	-----------------------------------------------------
	-- EXPORT	
	-----------------------------------------------------		
	sassystem_INCDIR = path.getabsolute("inc")
	
	configuration "macosx"
		sassystem_PLATFORM_INCDIR = path.getabsolute("inc/platform/posix/")
	configuration "windows"
		sassystem_PLATFORM_INCDIR = path.getabsolute("inc/platform/win32/")
	configuration {}
	
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
	