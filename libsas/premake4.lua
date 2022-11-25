project "libsas"
	uuid("34831360-bcdb-11e0-962b-0809500c9a43")
	kind "StaticLib"
	language "C++"  

	------------------------------------------------------
	-- CONFIG
	-----------------------------------------------------			
 
	if _OPTIONS["opengl"] then
		sasBACKEND = "opengl"
	else
		sasBACKEND = "dx11"
	end

	-----------------------------------------------------
	-- THIS
	-----------------------------------------------------			
	defines { "sasEXPORT" }
	includedirs { "inc/" }	
	includedirs { "src/" }		
	includedirs { "src/" }

	files { "inc/**.h", "src/**.cpp", "src/**.h" }

	configuration "opengl"
		defines {'PLATFORM_OPENGL'}
		excludes { "src/render/dx11/**"}
		includedirs { "src/render/opengl" }
	configuration "not opengl"
		defines {'PLATFORM_DX11'}
		excludes { "src/render/opengl/**"}
		includedirs { "src/render/dx11" }
	configuration "macosx"
		excludes { "src/platform/win32/**" }
		includedirs { "src/platform/posix/"}
		defines { 'sasBACKEND=\\"'..sasBACKEND..'\\"' }
		pchheader "libsas/src/sas_pch.h"
	configuration "windows"
		excludes { "src/platform/posix/**" }
		includedirs { "src/platform/win32/"}
		includedirs { ovr_INCDIR }
		libdirs		{ ovr_LIBDIR }
		defines { 'sasBACKEND=\"'..sasBACKEND..'\"' }
		pchheader "sas_pch.h"
		pchsource "src/sas_pch.cpp"
	configuration {"windows", "Debug"}
		links		{ "libovrd" }
    links   { "ws2_32" }
    links   { "winmm" }
	configuration {"windows", "Release"}
		links		{ "libovr" }
    links   { "ws2_32" }
    links   { "winmm" }
	configuration {}

	files { sas_SHADERDIR.."**.*" }	
	
	configuration "Debug"
		defines { "sasDEBUG" }
	configuration {}
	
	-----------------------------------------------------
	-- EXPORT	
	-----------------------------------------------------		
	libsas_INCDIR = path.getabsolute("inc")
	
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
	-- sasUtils
	-----------------------------------------------------
	
	includedirs { sasutils_INCDIR }
	includedirs { sasutils_PLATFORM_INCDIR }
	links 		{ "sasUtils" }
	
  
	-----------------------------------------------------
	-- D3D	
	-----------------------------------------------------
	configuration "not opengl"
		dxsdk_dir = os.getenv( "DXSDK_DIR" )	
  
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
	-- OPENGL	
	-----------------------------------------------------
	configuration {"opengl", "windows"}
		links {"opengl32", "glu32"}
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
	-- GLFW
	--------------------------------------------------------------------------------

	configuration "opengl"
		includedirs { glfw_INCDIR }	
		libdirs		{ glfw_LIBDIR }
		links 		{ "glfw" }
	configuration {"macosx", "opengl"}
		linkoptions { "-framework Cocoa", "-framework OpenGL" }
	configuration {}
	
	--------------------------------------------------------------------------------
	-- GLEW
	--------------------------------------------------------------------------------

	configuration "opengl"
		includedirs { glew_INCDIR }	
		libdirs		{ glew_LIBDIR }
	configuration {"windows", "opengl"}
		links 		{ "glew32" }
	configuration {"macosx", "opengl"}
		links 		{ "GLEW" }
	configuration {}