-- MySQL dump 10.13  Distrib 5.7.44, for Linux (x86_64)
--
-- Host: localhost    Database: Tasks
-- ------------------------------------------------------
-- Server version       5.7.44

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `ai3d_tasks`
--

DROP TABLE IF EXISTS `ai3d_tasks`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ai3d_tasks` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '自增主键',
  `user_id` varchar(64) NOT NULL COMMENT '用户ID',
  `tx_job_id` varchar(128) NOT NULL COMMENT '腾讯云任务ID',
  `request_id` varchar(128) DEFAULT NULL COMMENT '腾讯云请求ID',
  `status` varchar(32) DEFAULT NULL COMMENT '任务状态：QUEUING/RUNNING/SUCCEED/FAILED',
  `prompt` text COMMENT '提交的Prompt',
  `result_format` varchar(32) DEFAULT NULL COMMENT '期望产出格式',
  `error_message` text COMMENT '失败原因',
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  `deleted` tinyint(1) NOT NULL DEFAULT '0' COMMENT '软删 0-正常 1-删除',
  `version` varchar(45) NOT NULL DEFAULT 'comm',
  `downloadCount` int(11) NOT NULL DEFAULT '0',
  `Isprivate` tinyint(1) NOT NULL DEFAULT '0',
  `like` int(11) NOT NULL DEFAULT '0',
  `fileurl` varchar(45) DEFAULT NULL,
  `previewImages` varchar(45) DEFAULT NULL,
  `viewCount` int(11) DEFAULT '0' COMMENT '浏览量',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_tx_job_id` (`tx_job_id`),
  KEY `idx_user_ctime` (`user_id`,`create_time`),
  KEY `idx_ai3d_tasks_viewcount` (`viewCount`)
) ENGINE=InnoDB AUTO_INCREMENT=469 DEFAULT CHARSET=utf8mb4 COMMENT='AI3D 任务表';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ai3d_tasks`
--

LOCK TABLES `ai3d_tasks` WRITE;
/*!40000 ALTER TABLE `ai3d_tasks` DISABLE KEYS */;
INSERT INTO `ai3d_tasks` VALUES (110,'aff32f7c-9905-11f0-8927-00163e103396','1362713522949464064','f210c4a0-311f-4855-93b0-9cb12bc0738b','SUCCEED','一只猫','STL',NULL,'2025-09-25 16:57:01','2025-09-27 10:09:53',0,'rapid',0,0,2,'/models/stl/1758802152749_8461.stl','/models/stl/1758802152749_8461.jpg',0),(188,'ac65735e-983f-11f0-8927-00163e103396','job_1758712742243_3802','req_1758712742243_3802','SUCCEED','初始化文本','STL',NULL,'2025-09-28 20:29:48','2025-09-28 20:29:48',0,'comm',0,0,0,'/models/stl/1758712742243_3802.stl','/models/stl/1758712742243_3802.jpg',0),(189,'ac65735e-983f-11f0-8927-00163e103396','job_1758802152749_8461','req_1758802152749_8461','SUCCEED','初始化文本','STL',NULL,'2025-09-28 20:29:48','2025-09-28 20:29:48',0,'comm',0,0,0,'/models/stl/1758802152749_8461.stl','/models/stl/1758802152749_8461.jpg',0),(190,'ac65735e-983f-11f0-8927-00163e103396','job_1758979854622_7527','req_1758979854622_7527','SUCCEED','初始化文本','STL',NULL,'2025-09-28 20:29:48','2025-09-28 20:29:48',0,'comm',0,0,0,'/models/stl/1758979854622_7527.stl','/models/stl/1758979854622_7527.jpg',0),(191,'ac65735e-983f-11f0-8927-00163e103396','job_1758989671921_2981','req_1758989671921_2981','SUCCEED','初始化文本','STL',NULL,'2025-09-28 20:29:48','2025-09-28 20:29:48',0,'comm',0,0,0,'/models/stl/1758989671921_2981.stl','/models/stl/1758989671921_2981.jpg',0),(192,'ac65735e-983f-11f0-8927-00163e103396','job_1758989672533_2776','req_1758989672533_2776','SUCCEED','初始化文本','STL',NULL,'2025-09-28 20:29:48','2025-09-28 20:29:48',0,'comm',0,0,0,'/models/stl/1758989672533_2776.stl','/models/stl/1758989672533_2776.jpg',0),(423,'aff32f7c-9905-11f0-8927-00163e103396','job_1758712742243_3802_hou','req_1758712742243_3802_hou','SUCCEED','初始化文本','STL',NULL,'2025-09-28 20:42:13','2025-09-28 20:42:13',0,'comm',0,0,0,'/models/stl/1758712742243_3802.stl','/models/stl/1758712742243_3802.jpg',0),(424,'aff32f7c-9905-11f0-8927-00163e103396','job_1758802152749_8461_hou','req_1758802152749_8461_hou','SUCCEED','初始化文本','STL',NULL,'2025-09-28 20:42:13','2025-09-28 14:40:08',0,'comm',0,0,1,'/models/stl/1758802152749_8461.stl','/models/stl/1758802152749_8461.jpg',0),(425,'aff32f7c-9905-11f0-8927-00163e103396','job_1758979854622_7527_hou','req_1758979854622_7527_hou','SUCCEED','初始化文本','STL',NULL,'2025-09-28 20:42:13','2025-09-28 14:40:09',0,'comm',0,0,1,'/models/stl/1758979854622_7527.stl','/models/stl/1758979854622_7527.jpg',0),(426,'aff32f7c-9905-11f0-8927-00163e103396','job_1758989671921_2981_hou','req_1758989671921_2981_hou','SUCCEED','初始化文本','STL',NULL,'2025-09-28 20:42:13','2025-09-28 20:42:13',0,'comm',0,0,0,'/models/stl/1758989671921_2981.stl','/models/stl/1758989671921_2981.jpg',0),(427,'aff32f7c-9905-11f0-8927-00163e103396','job_1758989672533_2776_hou','req_1758989672533_2776_hou','SUCCEED','初始化文本','STL',NULL,'2025-09-28 20:42:13','2025-09-28 20:42:13',0,'comm',0,0,0,'/models/stl/1758989672533_2776.stl','/models/stl/1758989672533_2776.jpg',0),(428,'ac65735e-983f-11f0-8927-00163e103396','1363885683298926592','37052e66-5f2c-45e4-a7ef-6dc50dab0bc3','SUCCEED','一匹肌肉线条分明的成年骏马，通体覆盖柔顺光泽的栗色毛发，腹部渐变为浅米色，鼻梁与四蹄饰有细微白斑；皮肤纹理细腻，鬃毛随风轻扬呈现丝绒质感，在自然光线下毛发边缘泛着微弱高光；背景虚化突出主体，侧前方45度角构图，真实风格精准还原动物解剖结构与皮毛光影层次，真实材质, 写实风格，清晰轮廓, 适度细节','STL',NULL,'2025-09-28 14:34:46','2025-09-28 14:39:23',0,'rapid',1,0,0,'/models/stl/1759070187298_7595.stl','/models/stl/1759070187298_7595.jpg',0),(429,'ac65735e-983f-11f0-8927-00163e103396','1363886164423344128','e16a0e3a-a9ba-4c5a-b8df-cc0f68c350ee','SUCCEED','','STL',NULL,'2025-09-28 14:36:41','2025-09-28 14:37:46',0,'rapid',0,0,0,'/models/stl/1759070264833_2184.stl','/models/stl/1759070264833_2184.jpg',0),(430,'ac65735e-983f-11f0-8927-00163e103396','1363886704930725888','da7f1a04-4029-4ea9-8c8d-d64ef7d34efb','SUCCEED','一匹健壮的成年骏马呈站立姿态，肌肉线条流畅有力，棕褐色毛发带有自然光泽，腹部与四肢内侧毛色渐浅；皮肤纹理细腻，鼻尖湿润反光，鬃毛随风轻扬；写实风格精准还原解剖结构，柔和侧光凸显皮毛层次与体积感，深色背景衬托主体轮廓，突出静谧而生动的自然气息，真实材质, 写实风格，清晰轮廓, 适度细节','STL',NULL,'2025-09-28 14:38:50','2025-09-28 14:40:32',0,'rapid',0,0,0,'/models/stl/1759070430933_4719.stl','/models/stl/1759070430933_4719.jpg',0),(431,'ac65735e-983f-11f0-8927-00163e103396','1363887311800369152','0dcae8de-ef8e-48ac-9b10-3d88e41ed0a8','SUCCEED','','STL',NULL,'2025-09-28 14:41:14','2025-09-28 14:42:18',0,'rapid',0,0,0,'/models/stl/1759070535691_4250.stl','/models/stl/1759070535691_4250.jpg',0),(433,'ac65735e-983f-11f0-8927-00163e103396','1363893975379222528','24c6236b-ecf5-4fd4-bb3e-f91fe2f92434','SUCCEED','123','FBX',NULL,'2025-09-28 15:07:43','2025-09-28 15:13:22',0,'comm',0,0,0,'/models/fbx/1759072396913_2531.fbx','/models/fbx/1759072396913_2531.jpg',0),(434,'ac65735e-983f-11f0-8927-00163e103396','1363895824350052352','69d77aa8-c8db-46e5-a0b6-eeedcb7e90f8','SUCCEED','一匹马','USDZ',NULL,'2025-09-28 15:15:04','2025-09-28 15:18:49',0,'comm',0,0,0,'/models/usdz/1759072724874_2321.usdz','/models/usdz/1759072724874_2321.jpg',0),(435,'aff32f7c-9905-11f0-8927-00163e103396','1362713522949464064_copy1','f210c4a0-311f-4855-93b0-9cb12bc0738b_copy1','SUCCEED','-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'rapid',0,0,2,'/models/stl/1758802152749_8461.stl','/models/stl/1758802152749_8461.jpg',0),(436,'ac65735e-983f-11f0-8927-00163e103396','job_1758712742243_3802_copy1','req_1758712742243_3802_copy1','SUCCEED','-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'comm',0,0,0,'/models/stl/1758712742243_3802.stl','/models/stl/1758712742243_3802.jpg',0),(437,'ac65735e-983f-11f0-8927-00163e103396','job_1758802152749_8461_copy1','req_1758802152749_8461_copy1','SUCCEED','-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'comm',0,0,0,'/models/stl/1758802152749_8461.stl','/models/stl/1758802152749_8461.jpg',0),(438,'ac65735e-983f-11f0-8927-00163e103396','job_1758979854622_7527_copy1','req_1758979854622_7527_copy1','SUCCEED','-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'comm',0,0,0,'/models/stl/1758979854622_7527.stl','/models/stl/1758979854622_7527.jpg',0),(439,'ac65735e-983f-11f0-8927-00163e103396','job_1758989671921_2981_copy1','req_1758989671921_2981_copy1','SUCCEED','-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'comm',0,0,0,'/models/stl/1758989671921_2981.stl','/models/stl/1758989671921_2981.jpg',0),(440,'ac65735e-983f-11f0-8927-00163e103396','job_1758989672533_2776_copy1','req_1758989672533_2776_copy1','SUCCEED','-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'comm',0,0,0,'/models/stl/1758989672533_2776.stl','/models/stl/1758989672533_2776.jpg',0),(441,'aff32f7c-9905-11f0-8927-00163e103396','job_1758712742243_3802_hou_copy1','req_1758712742243_3802_hou_copy1','SUCCEED','-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'comm',0,0,0,'/models/stl/1758712742243_3802.stl','/models/stl/1758712742243_3802.jpg',0),(442,'aff32f7c-9905-11f0-8927-00163e103396','job_1758802152749_8461_hou_copy1','req_1758802152749_8461_hou_copy1','SUCCEED','-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'comm',0,0,1,'/models/stl/1758802152749_8461.stl','/models/stl/1758802152749_8461.jpg',0),(443,'aff32f7c-9905-11f0-8927-00163e103396','job_1758979854622_7527_hou_copy1','req_1758979854622_7527_hou_copy1','SUCCEED','-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'comm',0,0,1,'/models/stl/1758979854622_7527.stl','/models/stl/1758979854622_7527.jpg',0),(444,'aff32f7c-9905-11f0-8927-00163e103396','job_1758989671921_2981_hou_copy1','req_1758989671921_2981_hou_copy1','SUCCEED','-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'comm',0,0,0,'/models/stl/1758989671921_2981.stl','/models/stl/1758989671921_2981.jpg',0),(445,'aff32f7c-9905-11f0-8927-00163e103396','job_1758989672533_2776_hou_copy1','req_1758989672533_2776_hou_copy1','SUCCEED','-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'comm',0,0,0,'/models/stl/1758989672533_2776.stl','/models/stl/1758989672533_2776.jpg',0),(446,'ac65735e-983f-11f0-8927-00163e103396','1363885683298926592_copy1','37052e66-5f2c-45e4-a7ef-6dc50dab0bc3_copy1','SUCCEED','STL-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'rapid',1,0,0,'/models/stl/1759070187298_7595.stl','/models/stl/1759070187298_7595.jpg',0),(447,'ac65735e-983f-11f0-8927-00163e103396','1363886164423344128_copy1','e16a0e3a-a9ba-4c5a-b8df-cc0f68c350ee_copy1','SUCCEED','STL-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'rapid',0,0,0,'/models/stl/1759070264833_2184.stl','/models/stl/1759070264833_2184.jpg',0),(448,'ac65735e-983f-11f0-8927-00163e103396','1363886704930725888_copy1','da7f1a04-4029-4ea9-8c8d-d64ef7d34efb_copy1','SUCCEED','STL-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'rapid',0,0,0,'/models/stl/1759070430933_4719.stl','/models/stl/1759070430933_4719.jpg',0),(449,'ac65735e-983f-11f0-8927-00163e103396','1363887311800369152_copy1','0dcae8de-ef8e-48ac-9b10-3d88e41ed0a8_copy1','SUCCEED','STL-1','STL',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'rapid',0,0,0,'/models/stl/1759070535691_4250.stl','/models/stl/1759070535691_4250.jpg',0),(450,'ac65735e-983f-11f0-8927-00163e103396','1363893975379222528_copy1','24c6236b-ecf5-4fd4-bb3e-f91fe2f92434_copy1','SUCCEED','123-1','FBX',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'comm',0,0,0,'/models/fbx/1759072396913_2531.fbx','/models/fbx/1759072396913_2531.jpg',0),(451,'ac65735e-983f-11f0-8927-00163e103396','1363895824350052352_copy1','69d77aa8-c8db-46e5-a0b6-eeedcb7e90f8_copy1','RUN','USDZ-1','USDZ',NULL,'2025-09-28 15:17:35','2025-09-28 15:17:35',0,'comm',0,0,0,NULL,NULL,0),(452,'aff32f7c-9905-11f0-8927-00163e103396','1362713522949464064_copy2','f210c4a0-311f-4855-93b0-9cb12bc0738b_copy2','SUCCEED','-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'rapid',0,0,2,'/models/stl/1758802152749_8461.stl','/models/stl/1758802152749_8461.jpg',0),(453,'ac65735e-983f-11f0-8927-00163e103396','job_1758712742243_3802_copy2','req_1758712742243_3802_copy2','SUCCEED','-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'comm',0,0,0,'/models/stl/1758712742243_3802.stl','/models/stl/1758712742243_3802.jpg',0),(454,'ac65735e-983f-11f0-8927-00163e103396','job_1758802152749_8461_copy2','req_1758802152749_8461_copy2','SUCCEED','-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'comm',0,0,0,'/models/stl/1758802152749_8461.stl','/models/stl/1758802152749_8461.jpg',0),(455,'ac65735e-983f-11f0-8927-00163e103396','job_1758979854622_7527_copy2','req_1758979854622_7527_copy2','SUCCEED','-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'comm',0,0,0,'/models/stl/1758979854622_7527.stl','/models/stl/1758979854622_7527.jpg',0),(456,'ac65735e-983f-11f0-8927-00163e103396','job_1758989671921_2981_copy2','req_1758989671921_2981_copy2','SUCCEED','-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'comm',0,0,0,'/models/stl/1758989671921_2981.stl','/models/stl/1758989671921_2981.jpg',0),(457,'ac65735e-983f-11f0-8927-00163e103396','job_1758989672533_2776_copy2','req_1758989672533_2776_copy2','SUCCEED','-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'comm',0,0,0,'/models/stl/1758989672533_2776.stl','/models/stl/1758989672533_2776.jpg',0),(458,'aff32f7c-9905-11f0-8927-00163e103396','job_1758712742243_3802_hou_copy2','req_1758712742243_3802_hou_copy2','SUCCEED','-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'comm',0,0,0,'/models/stl/1758712742243_3802.stl','/models/stl/1758712742243_3802.jpg',0),(459,'aff32f7c-9905-11f0-8927-00163e103396','job_1758802152749_8461_hou_copy2','req_1758802152749_8461_hou_copy2','SUCCEED','-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'comm',0,0,1,'/models/stl/1758802152749_8461.stl','/models/stl/1758802152749_8461.jpg',0),(460,'aff32f7c-9905-11f0-8927-00163e103396','job_1758979854622_7527_hou_copy2','req_1758979854622_7527_hou_copy2','SUCCEED','-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'comm',0,0,1,'/models/stl/1758979854622_7527.stl','/models/stl/1758979854622_7527.jpg',0),(461,'aff32f7c-9905-11f0-8927-00163e103396','job_1758989671921_2981_hou_copy2','req_1758989671921_2981_hou_copy2','SUCCEED','-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'comm',0,0,0,'/models/stl/1758989671921_2981.stl','/models/stl/1758989671921_2981.jpg',0),(462,'aff32f7c-9905-11f0-8927-00163e103396','job_1758989672533_2776_hou_copy2','req_1758989672533_2776_hou_copy2','SUCCEED','-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'comm',0,0,0,'/models/stl/1758989672533_2776.stl','/models/stl/1758989672533_2776.jpg',0),(463,'ac65735e-983f-11f0-8927-00163e103396','1363885683298926592_copy2','37052e66-5f2c-45e4-a7ef-6dc50dab0bc3_copy2','SUCCEED','STL-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'rapid',1,0,0,'/models/stl/1759070187298_7595.stl','/models/stl/1759070187298_7595.jpg',0),(464,'ac65735e-983f-11f0-8927-00163e103396','1363886164423344128_copy2','e16a0e3a-a9ba-4c5a-b8df-cc0f68c350ee_copy2','SUCCEED','STL-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'rapid',0,0,0,'/models/stl/1759070264833_2184.stl','/models/stl/1759070264833_2184.jpg',0),(465,'ac65735e-983f-11f0-8927-00163e103396','1363886704930725888_copy2','da7f1a04-4029-4ea9-8c8d-d64ef7d34efb_copy2','SUCCEED','STL-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'rapid',0,0,0,'/models/stl/1759070430933_4719.stl','/models/stl/1759070430933_4719.jpg',0),(466,'ac65735e-983f-11f0-8927-00163e103396','1363887311800369152_copy2','0dcae8de-ef8e-48ac-9b10-3d88e41ed0a8_copy2','SUCCEED','STL-2','STL',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'rapid',0,0,0,'/models/stl/1759070535691_4250.stl','/models/stl/1759070535691_4250.jpg',0),(467,'ac65735e-983f-11f0-8927-00163e103396','1363893975379222528_copy2','24c6236b-ecf5-4fd4-bb3e-f91fe2f92434_copy2','SUCCEED','123-2','FBX',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'comm',0,0,0,'/models/fbx/1759072396913_2531.fbx','/models/fbx/1759072396913_2531.jpg',0),(468,'ac65735e-983f-11f0-8927-00163e103396','1363895824350052352_copy2','69d77aa8-c8db-46e5-a0b6-eeedcb7e90f8_copy2','RUN','USDZ-2','USDZ',NULL,'2025-09-28 15:17:44','2025-09-28 15:17:44',0,'comm',0,0,0,NULL,NULL,0);
/*!40000 ALTER TABLE `ai3d_tasks` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `daily_model_views`
--

DROP TABLE IF EXISTS `daily_model_views`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `daily_model_views` (
  `view_date` date NOT NULL COMMENT 'æ—¥æœŸ',
  `total_views` int(11) NOT NULL DEFAULT '0' COMMENT 'å½“å¤©æ€»æµè§ˆé‡',
  PRIMARY KEY (`view_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='æ¯æ—¥æ¨¡åž‹æµè§ˆæ€»è®¡';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `daily_model_views`
--

LOCK TABLES `daily_model_views` WRITE;
/*!40000 ALTER TABLE `daily_model_views` DISABLE KEYS */;
INSERT INTO `daily_model_views` VALUES ('2025-09-15',0),('2025-09-16',0),('2025-09-17',0),('2025-09-18',0),('2025-09-19',0),('2025-09-20',0),('2025-09-21',0),('2025-09-22',0),('2025-09-23',0),('2025-09-24',0),('2025-09-25',0),('2025-09-26',0),('2025-09-27',0),('2025-09-28',0);
/*!40000 ALTER TABLE `daily_model_views` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_model_downloads`
--

DROP TABLE IF EXISTS `user_model_downloads`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `user_model_downloads` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `user_id` varchar(64) NOT NULL COMMENT 'ç”¨æˆ·ID',
  `job_id` varchar(64) NOT NULL COMMENT 'ä»»åŠ¡/æ¨¡åž‹ID',
  `download_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'ä¸‹è½½æ—¶é—´',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uniq_user_job` (`user_id`,`job_id`) COMMENT 'ç¡®ä¿ä¸ªç”¨æˆ·å¯¹æ¯ä¸ªæ¨¡åž‹åªèƒ½ä¸‹è½½ä¸€æ¬¡',
  KEY `idx_user` (`user_id`) COMMENT 'ç”¨æˆ·ç´¢å¼•',
  KEY `idx_job` (`job_id`) COMMENT 'ä»»åŠ¡ç´¢å¼•',
  KEY `idx_download_time` (`download_time`) COMMENT 'ä¸‹è½½æ—¶é—´ç´¢å¼•'
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COMMENT='ç”¨æˆ·æ¨¡åž‹ä¸‹è½½è®°å½•è¡¨';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `user_model_downloads`
--

LOCK TABLES `user_model_downloads` WRITE;
/*!40000 ALTER TABLE `user_model_downloads` DISABLE KEYS */;
INSERT INTO `user_model_downloads` VALUES (1,'ac65735e-983f-11f0-8927-00163e103396','1363885683298926592','2025-09-28 14:39:23');
/*!40000 ALTER TABLE `user_model_downloads` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_model_likes`
--

DROP TABLE IF EXISTS `user_model_likes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `user_model_likes` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `user_id` varchar(64) NOT NULL,
  `job_id` varchar(64) NOT NULL,
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uniq_user_job` (`user_id`,`job_id`),
  KEY `idx_user` (`user_id`),
  KEY `idx_job` (`job_id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `user_model_likes`
--

LOCK TABLES `user_model_likes` WRITE;
/*!40000 ALTER TABLE `user_model_likes` DISABLE KEYS */;
INSERT INTO `user_model_likes` VALUES (2,'aff32f7c-9905-11f0-8927-00163e103396','1362763768278679552','2025-09-25 22:28:44'),(3,'ac65735e-983f-11f0-8927-00163e103396','job_1758990081550_7048_hou','2025-09-28 14:40:06'),(4,'ac65735e-983f-11f0-8927-00163e103396','job_1758802152749_8461_hou','2025-09-28 14:40:08'),(5,'ac65735e-983f-11f0-8927-00163e103396','job_1758979854622_7527_hou','2025-09-28 14:40:09');
/*!40000 ALTER TABLE `user_model_likes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_sessions`
--

DROP TABLE IF EXISTS `user_sessions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `user_sessions` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '自增主键',
  `user_id` varchar(64) NOT NULL COMMENT '用户唯一ID（关联 users.user_id）',
  `session_token` varchar(128) NOT NULL COMMENT '会话令牌（十六进制随机32字节）',
  `expire_time` datetime NOT NULL COMMENT '过期时间',
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `revoked` tinyint(1) NOT NULL DEFAULT '0' COMMENT '是否吊销',
  `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_session_token` (`session_token`),
  KEY `idx_user_expire` (`user_id`,`expire_time`),
  CONSTRAINT `fk_user_sessions_user_id` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4 COMMENT='用户会话表';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `user_sessions`
--

LOCK TABLES `user_sessions` WRITE;
/*!40000 ALTER TABLE `user_sessions` DISABLE KEYS */;
INSERT INTO `user_sessions` VALUES (1,'ac65735e-983f-11f0-8927-00163e103396','c5f0133d3a5ae87cfb3770a499b6c7745b0ce5d7a36333fdcf62242814ba3be6','2025-10-23 13:39:29','2025-09-23 13:39:29',0,'2025-09-28 14:31:58'),(4,'aff32f7c-9905-11f0-8927-00163e103396','aa2c5763429b50eb815b697d21efdc505c932d1a59c4d6d358bea063181db996','2025-10-24 13:16:57','2025-09-24 13:16:57',0,'2025-09-28 14:31:58');
/*!40000 ALTER TABLE `user_sessions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '自增主键',
  `user_id` varchar(64) NOT NULL COMMENT '用户唯一ID（UUID）',
  `username` varchar(64) NOT NULL COMMENT '用户名',
  `email` varchar(128) NOT NULL COMMENT '邮箱',
  `role` varchar(16) NOT NULL DEFAULT 'user',
  `token_count` int(11) NOT NULL DEFAULT '0',
  `password_hash` varchar(128) NOT NULL COMMENT '密码哈希（SHA256 十六进制）',
  `password_salt` varchar(64) NOT NULL COMMENT '盐（十六进制）',
  `status` tinyint(4) NOT NULL DEFAULT '1' COMMENT '1-正常 0-禁用',
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_user_id` (`user_id`),
  UNIQUE KEY `uk_username` (`username`),
  UNIQUE KEY `uk_email` (`email`)
) ENGINE=InnoDB AUTO_INCREMENT=33 DEFAULT CHARSET=utf8mb4 COMMENT='用户表';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `users`
--

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` VALUES (1,'ac65735e-983f-11f0-8927-00163e103396','lim','1811481344@qq.com','admin',5,'bc9b16d08d98f932ba5cff57284425d3e0209488709b8dee7c9e27f938e5aac4','e5d891fc407df9a58d65241ba4d8b5e9',1,'2025-09-23 13:39:29','2025-09-28 15:15:03'),(2,'aff32f7c-9905-11f0-8927-00163e103396','hou','2909307889@qq.com','admin',66,'9e22730c7709962e09ac95f09f5826152ba55fb97117c7235f65823f0629a0c5','84db3dc20435b74afbb1bb30e7533474',1,'2025-09-24 13:16:55','2025-09-25 21:40:57'),(3,'cd4ef30a-9c68-11f0-ba6a-00163e126023','testuser_1','testuser_1@example.com','user',100,'cde3c5302232d573b8a4d7fc88caa6a195ee5ddb73b21a88b5f1d8313ec82639','4b05a575b73fbd4354ec5304e5c1a6ff',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(4,'cd4ef843-9c68-11f0-ba6a-00163e126023','testuser_2','testuser_2@example.com','user',100,'d9c3b998714f15e267e530f8d818477fe0314af78024b6e709da1c426015b345','fd13886f3dcdc0e29a832e4697e5c237',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(5,'cd4ef98a-9c68-11f0-ba6a-00163e126023','testuser_3','testuser_3@example.com','user',100,'fe66ae46dcaf42fa85a15bc93bd639d0dc4f5319c6c8b12212773d1ec684c6b8','ff5b94cec4d0b92b4514edfccdfbe293',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(6,'cd4efa39-9c68-11f0-ba6a-00163e126023','testuser_4','testuser_4@example.com','user',100,'f6cb8faeba7e5108d8c11e07e9760fb7bd79f1f586f2c68c21f3d79feadec776','cfdb49cf37809f780fdd7d9658c125c0',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(7,'cd4efada-9c68-11f0-ba6a-00163e126023','testuser_5','testuser_5@example.com','user',100,'85021b55453627508f6803db98904dab77fe05aeeffe10d69e5ef8abb0ab30a2','9bf957ee767e523784223a6d5af82ead',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(8,'cd4efb81-9c68-11f0-ba6a-00163e126023','testuser_6','testuser_6@example.com','user',100,'bc1df1632b6f324f2fc553894b65d9f75c6828bc32246930d8fffd8b8d072fbe','1688f783fec3f10c316a197b2ce0b97d',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(9,'cd4efc7e-9c68-11f0-ba6a-00163e126023','testuser_7','testuser_7@example.com','user',100,'20c09fab46cd0d8e3ce0b7d7f4f7c5e9a8e7a12235ca280a9841619bcc5bdb36','54668ca4eba8d19855d19b20599fc3df',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(10,'cd4efd70-9c68-11f0-ba6a-00163e126023','testuser_8','testuser_8@example.com','user',100,'c1e8765457d2ad140eea3055ba69f6f807fa4b33fec4bcb3b2f5b6aad61e4dd0','ea98c3fe4d1dd10f85a6c3767c86c4a2',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(11,'cd4efe0e-9c68-11f0-ba6a-00163e126023','testuser_9','testuser_9@example.com','user',100,'0ae7bda14fb12eadc9f8ece9d63ed39a3ac2ce950d5a25933d3d9b43d55ec8ae','e57ef87bd91ce283293cc171c927f768',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(12,'cd4efea5-9c68-11f0-ba6a-00163e126023','testuser_10','testuser_10@example.com','user',100,'527563cff646814d1bfb53989abda1ccfc52642e73d981dfa623056c110a21a5','12ff93f8b4986c5aa5b30f984fdf7fdc',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(13,'cd4eff4c-9c68-11f0-ba6a-00163e126023','testuser_11','testuser_11@example.com','user',100,'3037bb7bcc5c77f6952f7d99271d1c61d4fc081afd5ce92ba9b958c2a1870c96','ca72b7402af4d0899bbb62e95d2e9734',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(14,'cd4efffa-9c68-11f0-ba6a-00163e126023','testuser_12','testuser_12@example.com','user',100,'12fe47f39debb42b52cc78db1b7c8f973cbc4e11c404d0b03a8413157b03d1a5','1c15d9a883baafd4cc4233d56d93f3bc',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(15,'cd4f00b2-9c68-11f0-ba6a-00163e126023','testuser_13','testuser_13@example.com','user',100,'cba76d19444841e9b07024af3b867409d6b489b7a000eafd1206d9e65c66bd95','50e856da187993ea935c073bb4747ecd',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(16,'cd4f01a6-9c68-11f0-ba6a-00163e126023','testuser_14','testuser_14@example.com','user',100,'20b26b90459a0b4b7ce3487a9f64e679dae66ce085c1a50b0f75dd7fa30856ea','842dfe579af952524002115460a457ef',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(17,'cd4f02ad-9c68-11f0-ba6a-00163e126023','testuser_15','testuser_15@example.com','user',100,'8b0bc962495622a289fb877d961d4864adbf5d4ca007605da932d37e97e2d23f','575d714d67574cbd22884fed81a981fd',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(18,'cd4f03ae-9c68-11f0-ba6a-00163e126023','testuser_16','testuser_16@example.com','user',100,'9394abb66390952537a9b94b23e017df0cea31953c38a89f421f9cc9ad5e25aa','810addd4c704ec520778ceebd70fcdc3',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(19,'cd4f0453-9c68-11f0-ba6a-00163e126023','testuser_17','testuser_17@example.com','user',100,'61dc1b4c21ff2cd70534a78de34a5d4cf525761184b6c78f3de5051b3f7951e0','9b84c9d8a2bd19765317adabeb43b674',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(20,'cd4f04ea-9c68-11f0-ba6a-00163e126023','testuser_18','testuser_18@example.com','user',100,'beda4253958d284b863eb4092bed812b307b433dff41d328ed9f59bd8dee0628','a187c4d7661fd20bc2217e942000f741',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(21,'cd4f05c4-9c68-11f0-ba6a-00163e126023','testuser_19','testuser_19@example.com','user',100,'dc4edb7d765463fca268a1b663dcbc71b5e292d268a00a453a88890444f86edb','a467e9bb7719ed5fbd4476f6cf3dab76',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(22,'cd4f0659-9c68-11f0-ba6a-00163e126023','testuser_20','testuser_20@example.com','user',100,'e7cceb8c3a2a989a1ad3f9665d6ee27d42d17328834fd043024fd50228ccbf20','cccfea0ae21d18080465db88a756b769',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(23,'cd4f0708-9c68-11f0-ba6a-00163e126023','testuser_21','testuser_21@example.com','user',100,'c69910c904d35d7bbf10150464cb9ed20f55fa55c7c495c00fd4f92d0a617325','0f71e583a7c2d20a7b4a7e4338ca4a22',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(24,'cd4f07ac-9c68-11f0-ba6a-00163e126023','testuser_22','testuser_22@example.com','user',100,'3ef72db8ef168c43891fab2f5e013094305c2da0fee52c828a2213013db585d6','24583f1d7208a36413c27025a3a8b43a',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(25,'cd4f084d-9c68-11f0-ba6a-00163e126023','testuser_23','testuser_23@example.com','user',100,'7897f62cc7c538efaae3c9cd741b76be4cbc5ebfae73f49f54774124f77c1577','0fb95c6c9685cf31479f3bac3559d564',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(26,'cd4f08f6-9c68-11f0-ba6a-00163e126023','testuser_24','testuser_24@example.com','user',100,'a77a61ebd2b14a85c2dd53fa6d3dea5372df1b17f56af294a34e54dce811a4c6','29ee06c945661390b5968ea30142dc50',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(27,'cd4f099a-9c68-11f0-ba6a-00163e126023','testuser_25','testuser_25@example.com','user',100,'a2f6616523fea5ec214e85275cdada4dcbb00803b97e98ebe802ac623811b52a','bb858b4f97f397e600aa76302992c15c',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(28,'cd4f0a39-9c68-11f0-ba6a-00163e126023','testuser_26','testuser_26@example.com','user',100,'afeb07d76e033fe7e5a0726999b00717fbddefdc347603809658fddf4aa2d2f2','0e4f9154d1dd9a247e94fb21f57ff6e7',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(29,'cd4f0acd-9c68-11f0-ba6a-00163e126023','testuser_27','testuser_27@example.com','user',100,'d8dae034208f21cff4df348ece10fcefe969703e99e5304a9c96497354f12f25','f1a8f0379c3459cec2b40783da3ee410',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(30,'cd4f0b5f-9c68-11f0-ba6a-00163e126023','testuser_28','testuser_28@example.com','user',100,'aed79a855797dc23488ae9ebc32265a605c43f49289fb6d656c7b2b7c4a74d0c','5e0ea73ac971f4a131d76298a1725076',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(31,'cd4f0bf1-9c68-11f0-ba6a-00163e126023','testuser_29','testuser_29@example.com','user',100,'7e94885fa026b45cb17f607f7baefcfaeafe4aa93e2952b3801d3d766b371104','0e31cd627669d8a39860d26265a9f554',1,'2025-09-28 20:43:58','2025-09-28 20:43:58'),(32,'cd4f0c86-9c68-11f0-ba6a-00163e126023','testuser_30','testuser_30@example.com','user',100,'a59313398c84bab619cbc8e63077b893a2377eb735da796a2d2646e4110730ef','6cf0cd754ea429113f2d633f21a41bb6',1,'2025-09-28 20:43:58','2025-09-28 20:43:58');
/*!40000 ALTER TABLE `users` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2025-09-28 15:20:17