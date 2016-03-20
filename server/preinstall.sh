sys=`uname -s`

cd ./deps
unzip include.zip
if [ "$sys" = "Darwin" ]; then
	unzip lib_darwin.zip;
fi
if [ "$sys" = "Linux" ]; then
	unzip lib_linux.zip;
fi
cd ..

if [ "$sys" = "Darwin" ]; then
	cp -f ./tools/premake4_darwin premake4;
	chmod -R +x *;
	./premake4 xcode4;
fi
if [ "$sys" = "Linux" ]; then
	cp -f ./tools/premake4_linux premake4;
	chmod -R +x *;
	./premake4 gmake;
fi

echo "安装完成～～～！"
