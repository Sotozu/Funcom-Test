project "sasMaths"
	uuid("34831360-bcdb-11e0-962b-f8f9f0fcfaf3")
	kind "StaticLib"
	language "C++"  	
	includedirs { "inc/" }	
	files { "inc/**.h", "src/**.cpp", "src/**.h" }			
	sasmaths_INCDIR = path.getabsolute("inc")