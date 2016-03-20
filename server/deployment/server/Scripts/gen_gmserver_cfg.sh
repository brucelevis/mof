#!/bin/sh

conn_str="tcp://"$gm_db_host":"$gm_db_port","$gm_db_user","$gm_db_pwd",Nx_GM"$server_id
server_ip=$gm_ip
server_port=$gm_port
thread_num=4
mod_id=4
pool_id=400

