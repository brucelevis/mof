#! /bin/sh
:<<up

up

#cd ./bin
#./stop_script


svn update
./cbp2make -in 'server.workspace' -out Makefile
rm -rf */obj
make

#cd ./sql_log
#mysql -f -uroot -proot -D mmorpg < 20120610.txt;

#cd ./bin
#./run_script_debug

