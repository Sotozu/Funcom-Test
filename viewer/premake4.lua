project "viewer"
	uuid("34831360-bcdb-11e0-962b-0826200c9a21")
	kind "ConsoleApp"
	language "C++"  

	configuration "opengl"
		files { "src/main_osx.cpp" }
	configuration "not opengl"
		files { "src/main.cpp", "src/sG/**.cpp", "src/sG/**.h" }
	configuration {}
	
	includedirs { sasmaths_INCDIR }
	links 		{ "sasMaths" }
	
	includedirs { sassystem_INCDIR }
	includedirs { sassystem_PLATFORM_INCDIR }
	links 		{ "sasSystem" }
	
	includedirs { sasutils_INCDIR }
	includedirs { sasutils_PLATFORM_INCDIR }
	links 		{ "sasUtils" }
	  
	includedirs { libsas_INCDIR }
	links { "libsas" }
	
	includedirs { "src/sG/" }
	
  -----------------------------------------------------
	-- AntTweakBar
	-----------------------------------------------------
	includedirs { anttweakbar_INCDIR }	
	libdirs		{ anttweakbar_LIBDIR }
	
	configuration "Debug"
		links 		{ "AntTweakBar_d" }		
	configuration "Release"
		links 		{ "AntTweakBar" }				
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
	
	
	--------------------------------------------------------------------------------
	-- GLFW
	--------------------------------------------------------------------------------
	
	configuration "Debug"
		defines { "sasDEBUG" }
	configuration "opengl"
		includedirs 	{ glfw_INCDIR }	
		includedirs 	{ glew_INCDIR }	
		libdirs		{ glfw_LIBDIR }
		includedirs 	{ anttweakbar_INCDIR }	
		libdirs		{ anttweakbar_LIBDIR }
		links 		{ "AntTweakBar" }
		links 		{ "glfw" }
	configuration {"windows", "opengl"}
		links       { "opengl32" }
	configuration "macosx"
		linkoptions 	{ "-framework Cocoa", "-framework OpenGL" }
		defines 	{ 'sas_RSCDIR=\\"'..sas_RSCDIR..'\\"' }
		defines 	{ 'sas_SHADERDIR=\\"'..sas_SHADERDIR..'\\"' }
	configuration "windows"
		defines 	{ 'sas_RSCDIR=\"'..sas_RSCDIR..'\"' }
		defines 	{ 'sas_SHADERDIR=\"'..sas_SHADERDIR..'\"' }
		dxsdk_dir = ""--os.getenv( "DXSDK_DIR" )	
		includedirs { path.join( dxsdk_dir, "Include" ) }	
		includedirs { ovr_INCDIR }	
		libdirs		{ path.join( dxsdk_dir, "Lib\\x86" ) }
		libdirs		{ ovr_LIBDIR }
		links 		{ "xinput" }
		links 		{ "winmm" }
		pchheader "sg_pch.h"
		pchsource "src/sG/sg_pch.cpp"
	configuration {"windows", "Debug"}
		links		{ "libovrd" }
    links   { "ws2_32" }
	configuration {"windows", "Release"}
		links		{ "libovr" }
    links   { "ws2_32" }
	configuration {}

  
  
  -----------------------------------------------------
	-- D3D	
	-----------------------------------------------------
	configuration "not opengl"
		dxsdk_dir = ""--os.getenv( "DXSDK_DIR" )	
  
		if( os.isdir( path.join( dxsdk_dir, "Include" ) ) == false ) then
			includedirs { "C:\\Program Files (x86)\\Windows Kits\\8.0\\Include\\um", "C:\\Program Files (x86)\\Windows Kits\\8.0\\Include\\shared" }	
			libdirs		{ "C:\\Program Files (x86)\\Windows Kits\\8.0\\Lib\\win8\\um\\x86" }
		else
			includedirs { path.join( dxsdk_dir, "Include" ) }	
			libdirs		{ path.join( dxsdk_dir, "Lib\\x86" ) }
		end
		links { "d3d11" }
		links { "dxgi" }
    
    if( os.isdir( path.join( dxsdk_dir, "Include" ) ) == false ) then
      links { "d3dcompiler" }
    else
      defines {'USE_D3DX'}
    end
    
	configuration {"not opengl", "Debug"}
		if( os.isdir( path.join( dxsdk_dir, "Include" ) ) == true ) then
      links 		{ "d3dx11d" }
    end
	configuration {"not opengl", "Release"}
		if( os.isdir( path.join( dxsdk_dir, "Include" ) ) == true ) then
      links 		{ "d3dx11" }
    end
	configuration {}


	-----------------------------------------------------
	-- ASSIMP	
	-----------------------------------------------------
	includedirs { assimp_INCDIR }	
	libdirs		{ assimp_LIBDIR }
	
	configuration "Debug"
		links 		{ "assimp_d" }
	configuration "Release"
		links 		{ "assimp" }
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
	
	--------------------------------------------------------------------------------
	-- DEVIL
	--------------------------------------------------------------------------------
	
	includedirs { devil_INCDIR }	
	libdirs		{ devil_LIBDIR }
  configuration "Debug"
		links 		{ "DevIL_d" }		
	configuration "Release"
		links 		{ "DevIL" }				
	configuration {}

	--------------------------------------------------------------------------------
	-- NVTT
	--------------------------------------------------------------------------------
	
	includedirs { nvtt_INCDIR }	
	libdirs		{ nvtt_LIBDIR }  
  configuration "Debug"
		links 		{ "nvtt_d" }		
	configuration "Release"
		links 		{ "nvtt" }				
	configuration {}
  
  --------------------------------------------------------------------------------
	-- FMOD
	--------------------------------------------------------------------------------
	
	includedirs { fmod_INCDIR }	
	libdirs		{ fmod_LIBDIR }  
  configuration "Debug"
		links 		{ "fmodL_vc" }		
	configuration "Release"
		links 		{ "fmod_vc" }				
	configuration {}

