#!/bin/sh
# 自动重启服务器脚本
#  crontab 添加脚本执行策略
#  修改
#
#
process_name="GameSrv10"
cd `dirname "$0"`
start_script="sh debug_run.sh"

#-----------------------------------
# 函数: CheckProcess# 功能: 检查一个进程是否存在
# 参数: $1 --- 要检查的进程名称# 返回: 如果存在返回0, 否则返回1.
#---------------------------------------
CheckProcess()
{   
	# 检查输入的参数是否有效   
	if [ "$1" = "" ];   
	then   
	  return 1   
	fi   
	#$PROCESS_NUM获取指定进程名的数目，为1返回0，表示正常，不为1返回1，表示有错误，需要重新启动   
	PROCESS_NUM=`ps -ef | grep "$1" | grep -v "grep" | wc -l`   
	if [ $PROCESS_NUM -eq 0 ];   
	then   
	    return 1   
	else   
		return 0   
	fi
}

# 检查实例是否已经
CheckProcess "$process_name"  
RET=$?  
if [ $RET -eq 1 ];  
then
	# 注意,这里运行的路径(若加到定时任务的话,需转到当前路径下)
	# 开启每个进程,可以打开的文件数(句柄)
	# ulimit -a
	# ulimit -n 200000
	# ./CrazyFort >> log.file 2>&1 &
	echo `date +%F\ %X` >> run.log
	echo "$start_script" >> run.log
	$start_script >> run.log
fi  
