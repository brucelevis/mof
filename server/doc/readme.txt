			项目说明书
文件目录说明
bin/config ;bin/lua ... 项目配置文件脚本启动脚本
bin/debug	debug二进制文件
bin/release 	release二进制文件
deps 底层库
common 客户端和服务器通用代码目录
obj 编译临时文件
run.sh 启动脚本
uninstall.sh 清除没用文件(备份)

外部开源库工具
premake4 跨平台生成工程工具


安装说明
cd deps 
./preinstall.sh 
提示安装完成 完成.


premake4使用
linux : ./premake4 gmake 
mac : ./premake4 xcode4
codeblock : ./premake4 codeblock
相对生成项目文件
 
