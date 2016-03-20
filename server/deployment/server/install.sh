#!/bin/sh

root_dir=`dirname $0`
final_dir="server"$server_id

. ./server_cfg.sh

server_dir=$root_dir
script_dir=$server_dir"/Scripts"
sql_dir=$server_dir"/Sql"
gameserver_dir=$server_dir"/GameServer"
mailserver_dir=$server_dir"/MailServer"
gmserver_dir=$server_dir"/GmServer"
echo $server_dir
echo $script_dir
echo $gameserver_dir
echo $mailserver_dir
echo $gmserver_dir


#开始生成配置文件
#生成邮件服务器的配置文件
cfg_file=PublicConfig.ini
echo "generate mail server config file"
. ./Scripts/gen_mailserver_cfg.sh
. ./Scripts/gen_public_cfg.sh
mv $cfg_file ./MailServer/

#生成GM服务器的配置文件
echo "generate gm server config file"
. ./Scripts/gen_gmserver_cfg.sh
. ./Scripts/gen_public_cfg.sh
mv $cfg_file ./GmServer/

#生成游戏服务器的配置文件
cfg_file=game.ini
echo "generate game server config file"
. ./Scripts/gen_gameserver_cfg.sh
mv $cfg_file ./GameServer/debug/
