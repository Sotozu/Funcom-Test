solution "jansson"
	location  ( "build/".._ACTION )
	targetdir ( "bin/".._ACTION )	

	configurations { "Debug", "Release" }
	
	defines { "_CRT_SECURE_NO_WARNINGS" }

	configuration "Debug"
		flags { "Symbols" }
		targetsuffix "_d"  
	configuration "Release"
		flags "Optimize"
	configuration {}

	project "jansson"	
		kind "StaticLib"
		language "C"
		files { "*.h", "*.c" }
		includedirs{ "." }


	
		
		


	
