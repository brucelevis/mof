solution"MergeDBWork"
	configurations {"Debug"}
	location "./"
	language "c++" 
	libdirs {"lib"}
project "MergeDB"
	location "./"
	kind "ConsoleApp"
	includedirs{"./", "lib", "Utils"}
	files{"./**.h","./**.cpp"}

configuration "Debug"
	targetdir "bin/debug"
	defines "DEBUG"
	flags {"Symbols"}
	libdirs {"bin/debug"}

configuration "vs*"
	defines {"_CRT_SECURE_NO_WARNINGS","WIN32"}

configuration "apple"
	links{"hiredis_apple"}

configuration "linux"
	links {"pthread","dl","rt","hiredis_linux"}

if _ACTION == "clean" then
	os.rmdir("bin/debug")
	os.rmdir("bin/release")
	os.rmdir("build/obj")
end	
