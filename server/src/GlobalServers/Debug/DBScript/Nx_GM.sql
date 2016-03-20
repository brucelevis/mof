-- --------------------------------------------------------
-- 主机:                           113.105.223.49
-- 服务器版本:                        5.5.33 - MySQL Community Server (GPL) by Remi
-- 服务器操作系统:                      Linux
-- HeidiSQL 版本:                  8.0.0.4396
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- 导出 Nx_GM1 的数据库结构
CREATE DATABASE IF NOT EXISTS `Nx_GM` /*!40100 DEFAULT CHARACTER SET utf8mb4 */;
USE `Nx_GM`;


-- 导出  过程 Nx_GM.Proc_GetGmTask 结构
DELIMITER //
CREATE DEFINER=`root`@`localhost` PROCEDURE `Proc_GetGmTask`()
BEGIN

declare _id INT;
declare _serverid INT;
declare _cmdstr VARCHAR(1024);

select id, serverid, cmdstr into _id, _serverid, _cmdstr from tb_gm_task  limit 1;

if _id is not null then
	delete from tb_gm_task where id = _id;
	select _serverid, _cmdstr;
end if;

END//
DELIMITER ;


-- 导出  表 Nx_GM.tb_gm_banchat 结构
CREATE TABLE IF NOT EXISTS `tb_gm_banchat` (
  `server_id` int(11) DEFAULT NULL,
  `role_id` int(11) DEFAULT NULL,
  `start_time` datetime DEFAULT NULL,
  `end_time` datetime DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- 数据导出被取消选择。


-- 导出  表 Nx_GM.tb_gm_banrole 结构
CREATE TABLE IF NOT EXISTS `tb_gm_banrole` (
  `server_id` int(11) DEFAULT NULL,
  `role_id` int(11) DEFAULT NULL,
  `start_time` datetime DEFAULT NULL,
  `end_time` datetime DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- 数据导出被取消选择。


-- 导出  表 Nx_GM.tb_gm_notify 结构
CREATE TABLE IF NOT EXISTS `tb_gm_notify` (
  `server_id` int(11) DEFAULT NULL,
  `notify_id` int(11) DEFAULT NULL,
  `content` varchar(1024) CHARACTER SET utf8 DEFAULT NULL,
  `start_time` datetime DEFAULT NULL,
  `end_time` datetime DEFAULT NULL,
  `send_interval` int(11) DEFAULT NULL,
  `total_times` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- 数据导出被取消选择。


-- 导出  表 Nx_GM.tb_gm_task 结构
CREATE TABLE IF NOT EXISTS `tb_gm_task` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `serverid` int(10) DEFAULT NULL,
  `cmdstr` varchar(1024) CHARACTER SET utf8 DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- 数据导出被取消选择。
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
