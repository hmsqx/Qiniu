-- 用户下载记录表
-- 用于跟踪每个用户对每个模型的下载情况，确保每个用户对每个模型只能计数一次
CREATE TABLE IF NOT EXISTS user_model_downloads (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  user_id VARCHAR(64) NOT NULL COMMENT '用户ID',
  job_id VARCHAR(64) NOT NULL COMMENT '任务/模型ID',
  download_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '下载时间',
  UNIQUE KEY uniq_user_job (user_id, job_id) COMMENT '确保每个用户对每个模型只能下载一次',
  INDEX idx_user (user_id) COMMENT '用户索引',
  INDEX idx_job (job_id) COMMENT '任务索引',
  INDEX idx_download_time (download_time) COMMENT '下载时间索引'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户模型下载记录表';



CREATE TABLE IF NOT EXISTS users (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '自增主键',
  user_id VARCHAR(64) NOT NULL COMMENT '用户唯一ID（UUID）',
  username VARCHAR(64) NOT NULL COMMENT '用户名',
  email VARCHAR(128) NOT NULL COMMENT '邮箱',
  role VARCHAR(16) NOT NULL DEFAULT 'user',
  token_count INT NOT NULL DEFAULT '0',
  password_hash VARCHAR(128) NOT NULL COMMENT '密码哈希（SHA256 十六进制）',
  password_salt VARCHAR(64) NOT NULL COMMENT '盐（十六进制）',
  status TINYINT NOT NULL DEFAULT '1' COMMENT '1-正常 0-禁用',
  create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (id),
  UNIQUE KEY uk_user_id (user_id) COMMENT '用户唯一ID唯一约束',
  UNIQUE KEY uk_username (username) COMMENT '用户名唯一约束',
  UNIQUE KEY uk_email (email) COMMENT '邮箱唯一约束'
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='用户表';


CREATE TABLE IF NOT EXISTS user_sessions (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '自增主键',
  user_id VARCHAR(64) NOT NULL COMMENT '用户唯一ID（关联 users.user_id）',
  session_token VARCHAR(128) NOT NULL COMMENT '会话令牌（十六进制随机32字节）',
  expire_time DATETIME NOT NULL COMMENT '过期时间',
  create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  revoked TINYINT(1) NOT NULL DEFAULT '0' COMMENT '是否吊销',
  update_time VARCHAR(45) NOT NULL,
  PRIMARY KEY (id),
  UNIQUE KEY uk_session_token (session_token) COMMENT '会话令牌唯一约束',
  KEY idx_user_expire (user_id, expire_time) COMMENT '用户ID+过期时间联合索引',
  CONSTRAINT fk_user_sessions_user_id FOREIGN KEY (user_id) REFERENCES users (user_id) COMMENT '关联用户表user_id外键'
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='用户会话表';

CREATE TABLE IF NOT EXISTS user_model_likes (
  id BIGINT NOT NULL AUTO_INCREMENT,
  user_id VARCHAR(64) NOT NULL,
  job_id VARCHAR(64) NOT NULL,
  create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  UNIQUE KEY uniq_user_job (user_id, job_id) COMMENT '确保每个用户对每个任务/模型只能点赞一次',
  KEY idx_user (user_id) COMMENT '用户ID索引',
  KEY idx_job (job_id) COMMENT '任务/模型ID索引'
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
CREATE TABLE IF NOT EXISTS ai3d_tasks (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '自增主键',
  user_id VARCHAR(64) NOT NULL COMMENT '用户ID',
  tx_job_id VARCHAR(128) NOT NULL COMMENT '腾讯云任务ID',
  request_id VARCHAR(128) DEFAULT NULL COMMENT '腾讯云请求ID',
  status VARCHAR(32) DEFAULT NULL COMMENT '任务状态：QUEUING/RUNNING/SUCCEED/FAILED',
  prompt TEXT COMMENT '提交的Prompt',
  result_format VARCHAR(32) DEFAULT NULL COMMENT '期望产出格式',
  error_message TEXT COMMENT '失败原因',
  create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  deleted TINYINT(1) NOT NULL DEFAULT '0' COMMENT '软删 0-正常 1-删除',
  version VARCHAR(45) NOT NULL DEFAULT 'comm',
  downloadCount INT NOT NULL DEFAULT '0',
  Isprivate TINYINT(1) NOT NULL DEFAULT '0',
  `like` INT NOT NULL DEFAULT '0',
  fileurl VARCHAR(45) DEFAULT NULL,
  previewImages VARCHAR(45) DEFAULT NULL,
  viewCount INT DEFAULT '0' COMMENT '浏览量',
  PRIMARY KEY (id),
  UNIQUE KEY uk_tx_job_id (tx_job_id) COMMENT '腾讯云任务ID唯一约束',
  KEY idx_user_ctime (user_id, create_time) COMMENT '用户ID+创建时间联合索引',
  KEY idx_ai3d_tasks_viewcount (viewCount) COMMENT '浏览量索引'
) ENGINE=InnoDB AUTO_INCREMENT=113 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='AI3D 任务表';