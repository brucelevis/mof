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

-- 导出 Nx_Mail 的数据库结构
CREATE DATABASE IF NOT EXISTS `Nx_Mail` /*!40100 DEFAULT CHARACTER SET utf8mb4 */;
USE `Nx_Mail`;


-- 导出  表 Nx_Mail.MailIndex 结构
CREATE TABLE IF NOT EXISTS `MailIndex` (
  `player` varchar(50) NOT NULL,
  PRIMARY KEY (`player`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 数据导出被取消选择。


-- 导出  表 Nx_Mail.MailTable 结构
CREATE TABLE IF NOT EXISTS `MailTable` (
  `MailID` char(40) NOT NULL DEFAULT '',
  `Owner` varchar(60) DEFAULT NULL,
  `Title` varchar(256) DEFAULT NULL,
  `Sender` varchar(60) DEFAULT NULL,
  `Receiver` varchar(60) DEFAULT NULL,
  `IsReaded` int(10) DEFAULT NULL,
  `SendTime` datetime DEFAULT NULL,
  `Attachment` varchar(4000) NOT NULL,
  `MailContent` varchar(4000) NOT NULL,
  PRIMARY KEY (`MailID`),
  KEY `Owner` (`Owner`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 数据导出被取消选择。


-- 导出  过程 Nx_Mail.Proc_MailAttachment 结构
DELIMITER //
CREATE DEFINER=`root`@`localhost` PROCEDURE `Proc_MailAttachment`(IN `__guid` char(40), IN `__attach` VARCHAR(4000), IN `__PlayerName` VARCHAR(60), IN `__result` INT)
BEGIN

	
	set __result = 1;
	set __attach = NULL;

	select  Attachment into __attach from MailTable where MailID = __guid 
		AND Receiver = __PlayerName AND Owner = __PlayerName;

	if __attach is not NULL AND length(__attach) > 0 THEN
		 set __result = 0;
	    update MailTable set Attachment = NULL where  MailID = __guid;
    end if;

	select __result,__attach;

END//
DELIMITER ;


-- 导出  过程 Nx_Mail.Proc_ReadMail 结构
DELIMITER //
CREATE DEFINER=`root`@`localhost` PROCEDURE `Proc_ReadMail`(IN `__guid` char(40), IN `__attach` VARCHAR(4000), IN `__content` VARCHAR(4000), IN `__result` INT)
BEGIN
   DECLARE __MailId char(40);
   DECLARE __isread INT;

	set __result = 1;
   set __content = NULL;
   set __attach = NULL;

	select  MailID, IsReaded, Attachment, MailContent into __MailId, __isread, __attach, __content from MailTable where MailID = __guid;

	if __MailId is not NULL THEN
			
			set __result = 0;

		   if __isread is NULL or __isread = 0 THEN
      		update MailTable set IsReaded = 1 where  MailID = __guid;
   		end IF;
		
	end IF;

	select __result, __content,__attach;

END//
DELIMITER ;


-- 导出  过程 Nx_Mail.Proc_SendMail 结构
DELIMITER //
CREATE DEFINER=`root`@`localhost` PROCEDURE `Proc_SendMail`(IN `__Owner` VARCHAR(60), IN `__Title` VARCHAR(256), IN `__Sender` VARCHAR(60), IN `__ReceiverName` VARCHAR(60), IN `__MailAttachment` VARCHAR(4000), IN `__MailContent` VARCHAR(4000))
BEGIN
	DECLARE __MailId char(40);
	set __MailId = uuid();
	
	insert into MailTable(MailID,Owner,Title,Sender,Receiver,SendTime,Attachment,MailContent)
				 values (__MailId,__Owner,__Title,__Sender,__ReceiverName,now(),__MailAttachment,__MailContent);
				 
	select __MailId;
END//
DELIMITER ;


-- 导出  表 Nx_Mail.SysListSendMail 结构
CREATE TABLE IF NOT EXISTS `SysListSendMail` (
  `Title` varchar(256) DEFAULT NULL,
  `Sender` varchar(60) DEFAULT NULL,
  `ReceiverList` tinytext,
  `SendTime` datetime DEFAULT NULL,
  `Attachment` varchar(512) NOT NULL,
  `MailContent` varchar(4000) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='群发邮件缓存';

-- 数据导出被取消选择。


-- 导出  表 Nx_Mail.SysMailStat 结构
CREATE TABLE IF NOT EXISTS `SysMailStat` (
  `MailID` char(40) DEFAULT NULL,
  `Owner` varchar(50) DEFAULT NULL,
  `SendTime` datetime DEFAULT NULL,
  `HasAttachment` int(10) DEFAULT NULL,
  `IsDelete` int(10) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='缓存邮件状态';

-- 数据导出被取消选择。


-- 导出  表 Nx_Mail.SysMailTable 结构
CREATE TABLE IF NOT EXISTS `SysMailTable` (
  `MailID` char(40) NOT NULL DEFAULT '',
  `Title` varchar(256) DEFAULT NULL,
  `SendTime` datetime DEFAULT NULL,
  `Attachment` varchar(512) NOT NULL,
  `MailContent` varchar(4000) NOT NULL,
  PRIMARY KEY (`MailID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='全服邮件缓存';

-- 数据导出被取消选择。
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
