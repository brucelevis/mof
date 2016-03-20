function deps_include() includedirs{"./deps/include","./deps/include/rapidxml", "./deps/include/lua"} end
solution "MobileServer"
	configurations {"Debug","Release"}
	location "build"
	language "C++"
	libdirs{"./deps/lib"}

project "GameSrv"
	location "build"
	kind "ConsoleApp"
	deps_include()
	includedirs{"src/common"}
	includedirs{"src/shared"}
	includedirs{"src/shared/base", "src/shared/crypto", "src/shared/db", "src/shared/mq", "src/shared/net", "src/shared/thread", "src/shared/conf", "src/shared/datapack"}
	includedirs{"src/framework"}
	includedirs{"src/servers/worldserver"}
	includedirs{"src/servers/worldserver/ai"}
	includedirs{"src/servers/worldserver/scene"}
	includedirs{"src/servers/logserver"}
	includedirs{"src/servers/gateserver"}
	includedirs{"src/GlobalServers", "../common", "../common/tinyxml", "src/GameSrv/activity","src/GameSrv/redishelper","src/GameSrv/operateact","src/GameSrv/boss","src/GameSrv/guild","src/GameSrv/publicserver", "src/GameSrv/sdk", "src/GameSrv", "src/GameSrv/net", "src/GameSrv/core", "src/GameSrv/cmd", "src/GameSrv/log", "src/GameSrv/global", "src/GameSrv/base", "src/GameSrv/world/", "src/GameSrv/world/scene/", "src/GameSrv/script", "src/GameSrv/world/ai", "src/GameSrv/systems", "src/GameSrv/config", "src/GameSrv/systems/daily_schedule"}
	files{"src/GameSrv/**.h","src/GameSrv/**.cpp"}
	files{"../common/**.h","../common/**.cpp"}
	files{"src/shared/**.h","src/shared/**.cpp"}
	files{"src/framework/**.h", "src/framework/**.cpp"}
	files{"src/servers/worldserver/**.h","src/servers/worldserver/**.cpp"}
	files{"src/servers/logserver/**.h","src/servers/logserver/**.cpp"}
	files{"src/servers/gateserver/**.h","src/servers/gateserver/**.cpp"}
	files{"src/common/**.h","src/common/**.cpp"}

configuration "Debug"
	targetdir "bin/debug"
	defines "DEBUG"
	flags {"Symbols"}
	libdirs {"./","./bin/debug"}
	links {"hiredis", "curl", "lua", "crypto", "uuid"}

configuration "linux"
	links {"pthread","dl","rt"}


if _ACTION == "clean" then
	os.rmdir("bin/debug")
	os.rmdir("bin/release")
	os.rmdir("build/obj")
end	
