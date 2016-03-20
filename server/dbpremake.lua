function deps_include() includedirs{"./deps/include","./deps/include/rapidxml", "./deps/include/lua"} end
solution "MobileServer"
	configurations {"Debug","Release"}
	location "build"
	language "C++"
	libdirs{"./deps/lib"}

project "db"
	location "build"
	kind "ConsoleApp"
	deps_include()
	includedirs{"src/db/"}
	files{"src/db/**.h","src/db/**.cpp"}

configuration "Debug"
	targetdir "bin/debug"
	defines "DEBUG"
	flags {"Symbols"}
	libdirs {"./","./bin/debug"}
	links {"hiredis", "curl", "lua", "crypto"}

configuration "Release"
	targetdir "bin/release"
	defines "NDEBUG"
	flags {"OptimizeSize"}
	libdirs {"./","./bin/release"}
	links {"hiredis", "curl"}

configuration "vs*"
	defines {"_CRT_SECURE_NO_WARNINGS","WIN32"}

configuration "linux"
	links {"pthread","dl","rt"}

if _ACTION == "clean" then
	os.rmdir("bin/debug")
	os.rmdir("bin/release")
	os.rmdir("build/obj")
end	
