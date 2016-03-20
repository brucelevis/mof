#!/bin/sh

URLHOST="http://113.105.223.34"

if [[ $# -lt 1 ]]; then
	echo "USAGE: `basename $0` [-v version] file file1 file2"
	exit 1
fi


while getopts :v: name
do
	case $name in
		v) 
			VERSION=$OPTARG
			;;
		?)
			echo "error"
			exit 1
			;;
	esac
done

URLDIR="${URLHOST}/server/versions/version${VERSION}/GameServer/"

for((i=$OPTIND; i<=$#; i++));do
	FILE=$(eval echo \${${i}})
	URL="${URLDIR}${FILE}"
	wget $URL
done
