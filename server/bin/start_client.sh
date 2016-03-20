#!/bin/sh

echo $#
echo $1" "$2

if [ $# -lt 2 ]; then
	echo "invalid arg num"
	exit 0
fi

for ((i = $1; i < $2; i++));
do
	echo $i
	nohup ./client -h 192.168.160.200 -a bat-$i &
done
