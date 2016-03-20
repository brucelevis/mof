#!/bin/sh
if [ "$#" = 0 ]; then
	echo "input the res package name"
	exit 1
fi

if [ "$1" != android ] && [ "$1" != ios ] && [ "$1" != lianyun ]; then
	echo "unknown package name"
	exit 1
fi

zipname=res_$1.zip
zip -r $zipname res -x "*.svn*" 
zip -r $zipname res/.svn/entries
