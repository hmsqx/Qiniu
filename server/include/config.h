#pragma once

// ---------------- Tencent Cloud Credentials ----------------
// IMPORTANT: Do not embed secrets at compile time. Runtime env should provide them.
// TENCENTCLOUD_SECRET_ID and TENCENTCLOUD_SECRET_KEY are intentionally not given defaults.
// Region can have a non-secret default.
#ifndef TENCENTCLOUD_REGION
#define TENCENTCLOUD_REGION "ap-guangzhou"
#endif

// ---------------- MySQL Connection ----------------
// Runtime env should provide DB params; defaults are non-secret and overridable at runtime.
// Kept here only as legacy fallbacks in some compilation branches.
#ifndef MYSQL_HOST
#define MYSQL_HOST "mysql"
#endif
#ifndef MYSQL_USER
#define MYSQL_USER "Qiniu"
#endif
#ifndef MYSQL_PASSWORD
#define MYSQL_PASSWORD ""
#endif
#ifndef MYSQL_DATABASE
#define MYSQL_DATABASE "Tasks"
#endif
#ifndef MYSQL_PORT
#define MYSQL_PORT 3306
#endif

// ---------------- Auth / Session ----------------
#ifndef SESSION_TTL_SECONDS
#define SESSION_TTL_SECONDS 2592000 // 30 天
#endif

// ---------------- AI3D Polling ----------------
#ifndef AI3D_POLL_INTERVAL_SECONDS
#define AI3D_POLL_INTERVAL_SECONDS 5
#endif

#ifndef AI3D_POLL_TIMEOUT_SECONDS
#define AI3D_POLL_TIMEOUT_SECONDS 900 // 15 分钟
#endif

// ---------------- Model Storage & Serving ----------------
#ifndef MODEL_FS_BASE_DIR
#define MODEL_FS_BASE_DIR "/var/www/models" // 模型文件物理存储根目录（绝对路径）
#endif

#ifndef MODEL_URL_BASE_PATH
#define MODEL_URL_BASE_PATH "/models" // 对外访问的URL前缀（由Nginx或本服务提供）
#endif

#ifndef MAX_CONCURRENT_MODEL_DOWNLOADS
#define MAX_CONCURRENT_MODEL_DOWNLOADS 4 // 并发下载线程数
#endif

#ifndef MODEL_DOWNLOAD_TIMEOUT_SECONDS
#define MODEL_DOWNLOAD_TIMEOUT_SECONDS 300 // 单个文件下载超时（秒）
#endif