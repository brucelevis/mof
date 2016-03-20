#!/bin/sh

cfg_file=PublicConfig.ini
conn_str="tcp://"$mail_db_host":"$mail_db_port","$mail_db_user","$mail_db_pwd",Nx_Mail"$server_id
server_ip=$mail_ip
server_port=$mail_port
thread_num=4
mod_id=2
pool_id=200
