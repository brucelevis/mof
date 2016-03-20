#!/bin/sh

echo "[system]" > $cfg_file
echo "ServerIP="$server_ip >> $cfg_file
echo "Port="$server_port >> $cfg_file

echo "" >> $cfg_file

echo "[db_conn_pool"$mod_id"]" >> $cfg_file
echo "mod_id="$mod_id >> $cfg_file
echo "pool_id="$pool_id >> $cfg_file
echo "threads="$thread_num >> $cfg_file
echo "conn="$conn_str >> $cfg_file
echo "role_range_from=1" >> $cfg_file
echo "role_range_to=100000000" >> $cfg_file
