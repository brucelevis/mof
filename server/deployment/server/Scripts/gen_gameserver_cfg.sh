#!/bin/sh

echo "[root]" > $cfg_file
echo "server_id="$server_id >> $cfg_file
echo "server_name="$server_name >> $cfg_file
echo "host="$game_host >> $cfg_file
echo "port="$game_port >> $cfg_file
echo "db_host="$game_db_host >> $cfg_file
echo "db_port="$game_db_port >> $cfg_file
echo "db_password="$game_db_password >> $cfg_file
echo "db_index="$game_db_index >> $cfg_file
echo "logpath="$game_log_path >> $cfg_file
echo "starttime="$game_start_time >> $cfg_file

echo "" >> $cfg_file

echo "[GmServer]" >> $cfg_file
echo "host=127.0.0.1" >> $cfg_file
echo "port="$gm_port >> $cfg_file

echo "" >> $cfg_file

echo "[MailServer]" >> $cfg_file
echo "host=127.0.0.1" >> $cfg_file
echo "port="$mail_port >> $cfg_file
