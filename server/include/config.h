#pragma once

// ---------------- Tencent Cloud Credentials ----------------
#ifndef TENCENTCLOUD_SECRET_ID
#define TENCENTCLOUD_SECRET_ID "YOUR_SECRET_ID"
#endif

#ifndef TENCENTCLOUD_SECRET_KEY
#define TENCENTCLOUD_SECRET_KEY "YOUR_SECRET_KEY"
#endif

#ifndef TENCENTCLOUD_REGION
#define TENCENTCLOUD_REGION "ap-guangzhou"
#endif

// ---------------- MySQL Connection ----------------
#ifndef MYSQL_HOST
#define MYSQL_HOST "127.0.0.1"
#endif

#ifndef MYSQL_USER
#define MYSQL_USER "Qiniu"
#endif

#ifndef MYSQL_PASSWORD
#define MYSQL_PASSWORD "Password"
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