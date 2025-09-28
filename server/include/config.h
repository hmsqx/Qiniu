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

// ---------------- API Security & Rate Limiting ----------------
#ifndef MAX_REQUESTS_PER_MINUTE
#define MAX_REQUESTS_PER_MINUTE 60 // 每分钟最大请求数
#endif

#ifndef MAX_REQUESTS_PER_HOUR
#define MAX_REQUESTS_PER_HOUR 1000 // 每小时最大请求数
#endif

#ifndef MAX_REQUESTS_PER_DAY
#define MAX_REQUESTS_PER_DAY 10000 // 每天最大请求数
#endif

#ifndef BURST_LIMIT
#define BURST_LIMIT 10 // 突发请求限制
#endif

#ifndef RATE_LIMIT_WINDOW_SECONDS
#define RATE_LIMIT_WINDOW_SECONDS 60 // 限流时间窗口大小（秒）
#endif

#ifndef MAX_REQUEST_SIZE_BYTES
#define MAX_REQUEST_SIZE_BYTES (1024 * 1024) // 最大请求体大小（1MB）
#endif

// ---------------- Server Configuration ----------------
#ifndef SERVER_PORT
#define SERVER_PORT 8080 // 服务器监听端口
#endif

#ifndef SERVER_HOST
#define SERVER_HOST "0.0.0.0" // 服务器监听地址
#endif


// ---------------- AI3D轮询配置 ----------------
#ifndef AI3D_POLL_INTERVAL_SECONDS
#define AI3D_POLL_INTERVAL_SECONDS 5
#endif

#ifndef AI3D_POLL_TIMEOUT_SECONDS
#define AI3D_POLL_TIMEOUT_SECONDS 900  // 15分钟
#endif