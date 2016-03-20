-- MySQL dump 10.13  Distrib 5.5.32, for Linux (x86_64)
--
-- Host: localhost    Database: Nx_Mail1001
-- ------------------------------------------------------
-- Server version	5.5.32

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `MailIndex`
--

/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `MailIndex` (
  `player` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  PRIMARY KEY (`player`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `MailTable`
--

/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `MailTable` (
  `MailID` char(40) NOT NULL DEFAULT '',
  `Owner` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `Title` varchar(256) DEFAULT NULL,
  `Sender` varchar(60) DEFAULT NULL,
  `Receiver` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `IsReaded` int(10) DEFAULT NULL,
  `SendTime` datetime DEFAULT NULL,
  `Attachment` varchar(4000) NOT NULL,
  `MailContent` varchar(4000) NOT NULL,
  PRIMARY KEY (`MailID`),
  KEY `Owner` (`Owner`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `SysListSendMail`
--

/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `SysListSendMail` (
  `Title` varchar(256) DEFAULT NULL,
  `Sender` varchar(60) DEFAULT NULL,
  `ReceiverList` tinytext,
  `SendTime` datetime DEFAULT NULL,
  `Attachment` varchar(512) NOT NULL,
  `MailContent` varchar(4000) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='群发邮件缓存';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `SysMailStat`
--

/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `SysMailStat` (
  `MailID` char(40) DEFAULT NULL,
  `Owner` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `SendTime` datetime DEFAULT NULL,
  `HasAttachment` int(10) DEFAULT NULL,
  `IsDelete` int(10) DEFAULT NULL,
  KEY `MailID` (`MailID`),
  KEY `Owner` (`Owner`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='缓存邮件状态';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `SysMailTable`
--

/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `SysMailTable` (
  `MailID` char(40) NOT NULL DEFAULT '',
  `Title` varchar(256) DEFAULT NULL,
  `SendTime` datetime DEFAULT NULL,
  `Attachment` varchar(512) NOT NULL,
  `MailContent` varchar(4000) NOT NULL,
  PRIMARY KEY (`MailID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='全服邮件缓存';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping routines for database 'Nx_Mail1001'
--
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_AUTO_VALUE_ON_ZERO' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `Proc_MailAttachment`(IN `__guid` char(40), IN `__attach` VARCHAR(4000), IN `__PlayerName` VARCHAR(60), IN `__result` INT)
BEGIN

	
	set __result = 1;
	set __attach = NULL;

	select  Attachment into __attach from MailTable where MailID = __guid 
		AND Receiver = __PlayerName AND Owner = __PlayerName;

	if __attach is not NULL AND length(__attach) > 0 THEN
		if row_count() = 1 then
			set __result = 0;
		else
		   set __result = 1;
		end if;
		update MailTable set Attachment = NULL where  MailID = __guid;
    end if;

	select __result,__attach;

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_AUTO_VALUE_ON_ZERO' */ ;
DELIMITER ;;
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

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'NO_AUTO_VALUE_ON_ZERO' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `Proc_SendMail`(IN `__Owner` VARCHAR(60), IN `__Title` VARCHAR(256), IN `__Sender` VARCHAR(60), IN `__ReceiverName` VARCHAR(60), IN `__MailAttachment` VARCHAR(4000), IN `__MailContent` VARCHAR(4000))
BEGIN
	DECLARE __MailId char(40);
	set __MailId = uuid();
	
	insert into MailTable(MailID,Owner,Title,Sender,Receiver,SendTime,Attachment,MailContent)
				 values (__MailId,__Owner,__Title,__Sender,__ReceiverName,now(),__MailAttachment,__MailContent);
				 
	select __MailId;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2013-11-18 11:29:47
