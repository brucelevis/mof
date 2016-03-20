#!/bin/sh

cd `dirname $0`
. ../server_cfg.sh

process_name="GmServer"
process_param=$server_id


GetPid()
{
	ps -ef | grep "$1\$" | grep -v "grep" | awk '{print $2}'
}

should_start=0

if [ $1 == "start" ];then
	should_start=1
elif [ $1 == "stop" ];then
	PID=`GetPid "${process_name} ${process_param}"`
	if [ $PID != "" ]; then
		kill $PID
	fi
elif [ $1 == "check" ];then
	PID=`GetPid "${process_name} ${process_param}"`
	if [ "$PID" == "" ]; then
		should_start=1
	fi
fi

if [ $should_start == 1 ];then
	echo `date +%F\ %X` >> run.log
	source /etc/profile
	chmod +x $process_name
	nohup ./$process_name $process_param &
fi

