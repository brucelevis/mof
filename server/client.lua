function deps_include() includedirs{"./deps/include","./deps/include/rapidxml", "./deps/include/lua"} end
solution "MobileServer"
	configurations {"Debug","Release"}
	location "build"
	language "C++"
	libdirs{"./deps/lib"}

project "client"
	location "build"
	kind "ConsoleApp"
	deps_include()
	includedirs{"src/client/"}
	includedirs{"../common/"}
	files{"src/client/**.h","src/client/**.cpp"}
	files{"../common/rc5.h","../common/rc5.cpp"}
	files{"../common/EasyEncrypt.h","../common/EasyEncrypt.cpp"}
	files{"../common/md5.h","../common/md5.cpp"}
	files{"../common/cyclemsg.h","../common/cyclemsg.cpp"}
	files{"../common/ByteArray.h","../common/ByteArray.cpp"}
	files{"../common/msg.h","../common/msg.cpp"}
	files{"../common/Utils.h","../common/Utils.cpp"}
	files{"../common/json/**.h","../common/json/**.cpp"}
	files{"../common/NetPacket.h","../common/NetPacket.cpp"}
	files{"../common/log.h","../common/log.cpp"}
	files{"../common/inifile.h","../common/inifile.cpp"}
	files{"../common/iniparser.h","../common/iniparser.cpp"}
	files{"../common/tee.h","../common/tee.cpp"}

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
