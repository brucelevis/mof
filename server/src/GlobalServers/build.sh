#! /bin/sh
:<<up

up


./cbp2make -in 'server.workspace' -out Makefile
rm -rf */obj
find . -name *.o | xargs rm -rf;
make

