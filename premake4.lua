--------------------------------------------------------------------------------
-- move_file
--------------------------------------------------------------------------------

function move_file( srcpath, dstfolder ) 
	
	if( os.isfile( srcpath ) == false ) then
		print( srcpath.." is not a file!" )
		return
	end
	
	if( os.isdir( dstfolder ) == false ) then
		print( dstfolder.." does not exist! Proceed to creation." )
		os.mkdir( dstfolder )
	end
	
	dstpath = path.join( dstfolder, path.getname( srcpath ) )		
	
	result = os.copyfile( srcpath, dstpath )
	if( result == nil ) then
		print( "Failed to copy "..srcpath.." in "..dstpath.."! Check that." )	
	end
end

--------------------------------------------------------------------------------
-- main
--------------------------------------------------------------------------------

solution "sas"
	location  ( "build/".._ACTION )
	targetdir ( "bin/".._ACTION )
	
	configurations { "Debug", "Release" }
	
	--------------------------------------------------------------------------------
	-- THIS
	--------------------------------------------------------------------------------
	
	configuration "Debug"
		flags { "Symbols" }
		targetsuffix "_d"  
	configuration "Release"
		flags { "Symbols" }
		flags "Optimize"
	configuration {}
	
	if(_ACTION == "gmake") then
		buildoptions { "-fasm-blocks" }
	else
		defines { "_CRT_SECURE_NO_WARNINGS" }	
	end

	flags{ "FatalWarnings" }

	sas_BINDIR = path.getabsolute("bin/".._ACTION)
	sas_RSCDIR = path.getabsolute("rsc")
	sas_RSCDIR = sas_RSCDIR..'/' 
	sas_SHADERDIR = sas_RSCDIR..'shaders/' 
	
	newoption {
		trigger     = "opengl",
		description = "Force the use of OpenGL for rendering, regardless of platform"
	}

	--------------------------------------------------------------------------------
	-- D3D
	--------------------------------------------------------------------------------
	
	d3d_DLLDIR = path.getabsolute( "external/dx/" ) .. "/"

	if os.get() == "windows" then
		move_file(  d3d_DLLDIR.."d3dcompiler_46.dll", sas_BINDIR )
		move_file(  d3d_DLLDIR.."xinput1_3.dll", sas_BINDIR )
	end
	
	--------------------------------------------------------------------------------
	-- ASSIMP
	--------------------------------------------------------------------------------
	assimp_INCDIR = path.getabsolute( "external/assimp/include" )
	assimp_LIBDIR = path.getabsolute( "external/assimp/lib/" )
	assimp_LIBDIR = assimp_LIBDIR.."/"
	
	if os.get() == "macosx" then
		move_file(  assimp_LIBDIR.."libassimp_d.dylib", sas_BINDIR )
		move_file(  assimp_LIBDIR.."libassimp.dylib", sas_BINDIR )
	elseif os.get() == "windows" then
		move_file(  assimp_LIBDIR.."assimp_d.dll", sas_BINDIR )
		move_file(  assimp_LIBDIR.."assimpd.pdb", sas_BINDIR )
		move_file(  assimp_LIBDIR.."assimp.dll", sas_BINDIR )
	end
	
	
	--------------------------------------------------------------------------------
	-- AntTweakBar
	--------------------------------------------------------------------------------
	
	anttweakbar_INCDIR = path.getabsolute( "external/anttweakbar/include/" )
	anttweakbar_LIBDIR = path.getabsolute( "external/anttweakbar/lib/" ) .. "/"
	anttweakbar_DLLDIR = path.getabsolute( "external/anttweakbar/lib/" ) .. "/"

	if os.get() == "macosx" then
		move_file(  anttweakbar_DLLDIR.."libAntTweakBar.dylib", sas_BINDIR )
	elseif os.get() == "windows" then
		move_file(  anttweakbar_DLLDIR.."AntTweakBar.dll", sas_BINDIR )
		move_file(  anttweakbar_DLLDIR.."AntTweakBar.pdb", sas_BINDIR )
		move_file(  anttweakbar_DLLDIR.."AntTweakBar_d.pdb", sas_BINDIR )
    move_file(  anttweakbar_DLLDIR.."AntTweakBar_d.dll", sas_BINDIR )
	end
	
	--------------------------------------------------------------------------------
	-- JANSSON
	--------------------------------------------------------------------------------
	
	jansson_INCDIR = path.getabsolute( "external/jansson/inc" )
	jansson_LIBDIR = path.getabsolute( "external/jansson/lib/" )
	jansson_LIBDIR = jansson_LIBDIR.."/"

	if os.get() == "macosx" then
		move_file(  jansson_LIBDIR.."libjansson_d.dylib", sas_BINDIR )
		move_file(  jansson_LIBDIR.."libjansson.dylib", sas_BINDIR )
	end
	
	--------------------------------------------------------------------------------
	-- DEVIL
	--------------------------------------------------------------------------------
	
	devil_INCDIR = path.getabsolute( "external/devil/inc" )
	devil_LIBDIR = path.getabsolute( "external/devil/lib/" ) .. "/"
	devil_LIBDIR = devil_LIBDIR.."/"

	if os.get() == "macosx" then
		move_file(  devil_LIBDIR.."libIL.1.dylib", sas_BINDIR )
	elseif os.get() == "windows" then
		move_file(  devil_LIBDIR.."DevIL.dll", sas_BINDIR )
    move_file(  devil_LIBDIR.."DevIL_d.dll", sas_BINDIR )
    move_file(  devil_LIBDIR.."DevIL_d.pdb", sas_BINDIR )
    move_file(  devil_LIBDIR.."zlib_d.dll", sas_BINDIR )
    move_file(  devil_LIBDIR.."zlib1.dll", sas_BINDIR )
	end
	
	--------------------------------------------------------------------------------
	-- NVTT
	--------------------------------------------------------------------------------
	
	nvtt_INCDIR = path.getabsolute( "external/nvtt/inc" )
	nvtt_LIBDIR = path.getabsolute( "external/nvtt/lib/" ) .. "/"
	nvtt_LIBDIR = nvtt_LIBDIR.."/"

	if os.get() == "windows" then
		move_file(  nvtt_LIBDIR.."nvtt.dll", sas_BINDIR )
    move_file(  nvtt_LIBDIR.."nvtt_d.dll", sas_BINDIR )
    move_file(  nvtt_LIBDIR.."jpeg62.dll", sas_BINDIR )
    move_file(  nvtt_LIBDIR.."libpng12.dll", sas_BINDIR )
    move_file(  nvtt_LIBDIR.."libtiff3.dll", sas_BINDIR )
	end

	--------------------------------------------------------------------------------
	-- GLFW
	--------------------------------------------------------------------------------
	if _OPTIONS["opengl"] then
		glfw_INCDIR = path.getabsolute( "external/glfw-2.7.2/include/" )
		glfw_LIBDIR = path.getabsolute( "external/glfw-2.7.2/lib/" )
		glfw_LIBDIR = glfw_LIBDIR.."/"

    	if os.get() == "macosx" then
    		move_file(  glfw_LIBDIR.."libglfw.dylib", sas_BINDIR )
    	end
	end

	--------------------------------------------------------------------------------
	-- GLEW
	--------------------------------------------------------------------------------
    if _OPTIONS["opengl"] then
	    glew_INCDIR = path.getabsolute( "external/glew-1.7.0/include/" )
	    glew_LIBDIR = path.getabsolute( "external/glew-1.7.0/lib/" )
	    glew_LIBDIR = glew_LIBDIR.."/"

	    if os.get() == "windows" then
		    move_file(  glew_LIBDIR.."glew32.dll", sas_BINDIR )
	    elseif os.get() == "macosx" then
		    move_file(  glew_LIBDIR.."libGLEW.1.7.0.dylib", sas_BINDIR )
	    end
    end
    
	--------------------------------------------------------------------------------
	-- OVR
	--------------------------------------------------------------------------------
	ovr_INCDIR = path.getabsolute( "external/libovr-0.4.4/Include/" )
	ovr_LIBDIR = path.getabsolute( "external/libovr-0.4.4/Lib/Win32/" ) .. "/"
	ovr_LIBDIR = ovr_LIBDIR.."/"
  
  --------------------------------------------------------------------------------
	-- FMOD Studio 
	--------------------------------------------------------------------------------
	fmod_INCDIR = path.getabsolute( "external/fmod/1.06.01/inc/" )
	fmod_LIBDIR = path.getabsolute( "external/fmod/1.06.01/lib/" )	
	fmod_LIBDIR = fmod_LIBDIR.."/"
  if os.get() == "windows" then
		move_file(  fmod_LIBDIR.."fmod.dll", sas_BINDIR )
    move_file(  fmod_LIBDIR.."fmodL.dll", sas_BINDIR )
	end
	
	--------------------------------------------------------------------------------
	-- INCLUDES
	--------------------------------------------------------------------------------
	
	include "sasmaths"
	include "sassystem"
	include "sasutils"
	include "libsas"
	include "viewer"

