
# AI3D C++ 后端服务器
# 1. 克隆项目
git clone <repository-url>
cd Qiniu/server

# 2. 一键部署
sudo ./deploy.sh

# 3. 配置数据库和密钥
nano include/config.h

# 4. 启动服务
systemctl start ai3d-server

# 5. 健康检查
./health_check.sh
基于C++和httplib构建的高性能AI3D模型生成服务后端，提供完整的用户管理、模型管理、文件服务等功能。

## 🚀 服务器特性

### 核心功能
- **AI3D模型生成**：集成腾讯云AI3D SDK，支持文本到3D模型转换
- **用户系统**：完整的注册、登录、会话管理
- **模型管理**：模型上传、下载、预览、收藏、统计
- **文件服务**：静态文件托管，支持多种3D格式
- **权限控制**：基于角色的访问控制，支持私有/公开模型
- **API限流**：IP限流、请求频率控制
- **安全防护**：SQL注入防护、XSS防护、会话安全

### 技术架构
- **Web框架**：httplib (轻量级HTTP服务器)
- **数据库**：MySQL 8.0+ (连接池管理)
- **AI服务**：腾讯云AI3D SDK
- **并发处理**：线程池 + 异步任务
- **安全机制**：多层安全中间件
- **配置管理**：编译时宏配置

## 📁 项目结构

```
server/
├── src/                    # 源代码
│   ├── main.cc            # 主程序入口
│   ├── handlers.cc        # HTTP请求处理器
│   ├── db_utils.cc        # 数据库操作工具
│   ├── auth.cc            # 用户认证模块
│   ├── tx_ai3d.cc         # 腾讯云AI3D集成
│   └── ...
├── include/               # 头文件
│   ├── handlers.h         # 处理器声明
│   ├── db_utils.h         # 数据库工具声明
│   ├── auth.h             # 认证模块声明
│   ├── config.h           # 配置宏定义
│   └── ...
├── build/                 # 编译输出目录
├── CMakeLists.txt         # CMake构建配置
├── database_migration.sql # 数据库结构
├── database_migration_download_fix.sql # 下载功能修复
└── build.sh              # 构建脚本
```

## 🛠️ 环境要求

### 系统要求
- **操作系统**：Linux (Ubuntu 18.04+ 推荐)
- **内存**：至少 2GB RAM
- **存储**：至少 10GB 可用空间
- **CPU**：2核心以上

### 依赖软件
- **编译器**：GCC 7.0+ (支持C++17)
- **构建工具**：CMake 3.1+
- **数据库**：MySQL 5.7+ 或 MariaDB 10.3+
- **系统库**：
  - libmysqlclient-dev
  - libcurl4-openssl-dev
  - libssl-dev
  - jsoncpp

## 🔧 安装和配置

### 1. 安装依赖

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install -y build-essential cmake libmysqlclient-dev libcurl4-openssl-dev libssl-dev libjsoncpp-dev
```

**CentOS/RHEL:**
```bash
sudo yum install -y gcc-c++ cmake mysql-devel libcurl-devel openssl-devel jsoncpp-devel
```

### 2. 数据库配置

```sql
-- 创建数据库
CREATE DATABASE Tasks CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 创建用户
CREATE USER 'ai3d_user'@'localhost' IDENTIFIED BY 'your_password';
GRANT ALL PRIVILEGES ON Tasks.* TO 'ai3d_user'@'localhost';
FLUSH PRIVILEGES;

-- 导入数据库结构
mysql -u ai3d_user -p Tasks < database_migration.sql
mysql -u ai3d_user -p Tasks < database_migration_download_fix.sql
```

### 3. 配置文件

编辑 `include/config.h` 文件：

```cpp
#pragma once

// ---------------- 腾讯云配置 ----------------
#ifndef TENCENTCLOUD_SECRET_ID
#define TENCENTCLOUD_SECRET_ID "your_secret_id"
#endif

#ifndef TENCENTCLOUD_SECRET_KEY
#define TENCENTCLOUD_SECRET_KEY "your_secret_key"
#endif

#ifndef TENCENTCLOUD_REGION
#define TENCENTCLOUD_REGION "ap-guangzhou"
#endif

// ---------------- 数据库配置 ----------------
#ifndef MYSQL_HOST
#define MYSQL_HOST "localhost"
#endif

#ifndef MYSQL_USER
#define MYSQL_USER "ai3d_user"
#endif

#ifndef MYSQL_PASSWORD
#define MYSQL_PASSWORD "your_password"
#endif

#ifndef MYSQL_DATABASE
#define MYSQL_DATABASE "Tasks"
#endif

#ifndef MYSQL_PORT
#define MYSQL_PORT 3306
#endif

// ---------------- 服务器配置 ----------------
#ifndef SERVER_PORT
#define SERVER_PORT 8080
#endif

#ifndef SERVER_HOST
#define SERVER_HOST "0.0.0.0"
#endif

// ---------------- 会话管理 ----------------
#ifndef SESSION_TTL_SECONDS
#define SESSION_TTL_SECONDS 2592000  // 30天
#endif

// ---------------- AI3D轮询配置 ----------------
#ifndef AI3D_POLL_INTERVAL_SECONDS
#define AI3D_POLL_INTERVAL_SECONDS 5
#endif

#ifndef AI3D_POLL_TIMEOUT_SECONDS
#define AI3D_POLL_TIMEOUT_SECONDS 900  // 15分钟
#endif

// ---------------- 模型存储配置 ----------------
#ifndef MODEL_FS_BASE_DIR
#define MODEL_FS_BASE_DIR "/var/www/models"
#endif

#ifndef MODEL_URL_BASE_PATH
#define MODEL_URL_BASE_PATH "/models"
#endif

#ifndef MAX_CONCURRENT_MODEL_DOWNLOADS
#define MAX_CONCURRENT_MODEL_DOWNLOADS 4
#endif

#ifndef MODEL_DOWNLOAD_TIMEOUT_SECONDS
#define MODEL_DOWNLOAD_TIMEOUT_SECONDS 300
#endif

// ---------------- API安全配置 ----------------
#ifndef MAX_REQUESTS_PER_MINUTE
#define MAX_REQUESTS_PER_MINUTE 60
#endif

#ifndef MAX_REQUESTS_PER_HOUR
#define MAX_REQUESTS_PER_HOUR 1000
#endif

#ifndef MAX_REQUESTS_PER_DAY
#define MAX_REQUESTS_PER_DAY 10000
#endif

#ifndef BURST_LIMIT
#define BURST_LIMIT 10
#endif

#ifndef RATE_LIMIT_WINDOW_SECONDS
#define RATE_LIMIT_WINDOW_SECONDS 60
#endif

#ifndef MAX_REQUEST_SIZE_BYTES
#define MAX_REQUEST_SIZE_BYTES (1024 * 1024)  // 1MB
#endif
```

### 4. 编译服务器

```bash
cd server
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

### 5. 创建必要目录

```bash
# 创建模型存储目录
sudo mkdir -p /var/www/models
sudo chown -R www-data:www-data /var/www/models
sudo chmod -R 755 /var/www/models

# 创建日志目录
sudo mkdir -p /var/log/ai3d
sudo chown -R www-data:www-data /var/log/ai3d
```

## 🚀 启动服务

### 开发模式
```bash
cd server/build
./tencent_cloud_cpp_sample
```

### 生产模式
```bash
# 后台运行
cd server/build
nohup ./tencent_cloud_cpp_sample > /var/log/ai3d/server.log 2>&1 &

# 查看日志
tail -f /var/log/ai3d/server.log
```

### 系统服务
```bash
# 创建systemd服务文件
sudo tee /etc/systemd/system/ai3d-server.service > /dev/null <<EOF
[Unit]
Description=AI3D Server
After=network.target mysql.service

[Service]
Type=simple
User=www-data
WorkingDirectory=/root/Qiniu/server/build
ExecStart=/root/Qiniu/server/build/tencent_cloud_cpp_sample
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

# 启动服务
sudo systemctl daemon-reload
sudo systemctl enable ai3d-server
sudo systemctl start ai3d-server

# 查看状态
sudo systemctl status ai3d-server
```

## 📊 API接口

### 用户认证
- `POST /api/register` - 用户注册
- `POST /api/login` - 用户登录
- `POST /api/logout` - 用户登出
- `GET /api/auth/me` - 获取当前用户信息

### 模型管理
- `POST /api/get_model` - 生成3D模型
- `GET /api/query` - 查询模型列表（分页）
- `POST /api/downloadModel` - 下载模型（计数）
- `POST /api/like` - 点赞/取消点赞
- `GET /api/showModel` - 查看模型详情

### 文件服务
- `GET /models/*` - 模型文件下载
- `GET /download/*` - 文件下载

### 管理员接口
- `GET /api/admin/overview` - 系统概览统计
- `GET /api/admin/users` - 用户管理
- `GET /api/admin/models` - 模型管理

### 健康检查
- `GET /health` - 服务健康检查

## 🔒 安全特性

### 认证和授权
- **会话管理**：基于token的会话机制
- **密码安全**：SHA256+盐值哈希存储
- **会话过期**：自动清理过期会话
- **权限控制**：基于角色的访问控制

### API安全
- **IP限流**：防止恶意请求
- **请求验证**：JSON格式验证
- **SQL注入防护**：参数转义和预处理
- **XSS防护**：输入过滤和输出编码

### 数据安全
- **连接池**：数据库连接复用
- **事务安全**：关键操作事务保护
- **并发控制**：线程安全的数据访问

## ⚡ 性能优化

### 配置调优

**小型部署（<1000用户）：**
```cpp
#define MAX_CONCURRENT_MODEL_DOWNLOADS 4
#define MAX_REQUESTS_PER_MINUTE 60
```

**中型部署（1000-10000用户）：**
```cpp
#define MAX_CONCURRENT_MODEL_DOWNLOADS 8
#define MAX_REQUESTS_PER_MINUTE 100
```

**大型部署（>10000用户）：**
```cpp
#define MAX_CONCURRENT_MODEL_DOWNLOADS 16
#define MAX_REQUESTS_PER_MINUTE 200
```

### 数据库优化
```sql
-- 添加索引
CREATE INDEX idx_ai3d_tasks_user_id ON ai3d_tasks(user_id);
CREATE INDEX idx_ai3d_tasks_status ON ai3d_tasks(status);
CREATE INDEX idx_ai3d_tasks_create_time ON ai3d_tasks(create_time);
CREATE INDEX idx_user_sessions_token ON user_sessions(session_token);
CREATE INDEX idx_user_sessions_expire ON user_sessions(expire_time);
```

### 系统优化
```bash
# 增加文件描述符限制
echo "* soft nofile 65535" >> /etc/security/limits.conf
echo "* hard nofile 65535" >> /etc/security/limits.conf

# 优化网络参数
echo "net.core.somaxconn = 65535" >> /etc/sysctl.conf
echo "net.ipv4.tcp_max_syn_backlog = 65535" >> /etc/sysctl.conf
sysctl -p
```

## 📈 监控和日志

### 日志配置
```bash
# 查看服务日志
journalctl -u ai3d-server -f

# 查看应用日志
tail -f /var/log/ai3d/server.log
```

### 性能监控
```bash
# 检查服务状态
curl http://localhost:8080/health

# 检查端口占用
netstat -tulpn | grep :8080

# 检查进程资源使用
top -p $(pgrep tencent_cloud_cpp_sample)
```

### 健康检查脚本
```bash
#!/bin/bash
# health_check.sh

if ! curl -f http://localhost:8080/health > /dev/null 2>&1; then
    echo "Service is down, restarting..."
    sudo systemctl restart ai3d-server
    sleep 10
    
    if ! curl -f http://localhost:8080/health > /dev/null 2>&1; then
        echo "Service restart failed, sending alert..."
        # 发送告警
    fi
fi
```

## 🔄 维护和更新

### 备份策略
```bash
# 数据库备份
mysqldump -u ai3d_user -p Tasks > backup_$(date +%Y%m%d).sql

# 配置文件备份
cp include/config.h backup/config_$(date +%Y%m%d).h

# 模型文件备份
tar -czf models_backup_$(date +%Y%m%d).tar.gz /var/www/models/
```

### 更新流程
```bash
# 1. 备份当前版本
cp build/tencent_cloud_cpp_sample build/tencent_cloud_cpp_sample.backup

# 2. 拉取最新代码
git pull origin main

# 3. 重新编译
cd build && make -j$(nproc)

# 4. 重启服务
sudo systemctl restart ai3d-server

# 5. 验证服务
curl http://localhost:8080/health
```

### 定期维护
```bash
# 创建维护脚本
cat > /root/maintenance.sh << 'EOF'
#!/bin/bash

# 清理日志文件
find /var/log -name "*.log" -mtime +30 -delete

# 数据库维护
mysql -u root -p -e "OPTIMIZE TABLE ai3d_tasks;"

# 重启服务（如果需要）
# systemctl restart ai3d-server

echo "维护任务完成: $(date)"
EOF

chmod +x /root/maintenance.sh

# 添加到crontab（每周日凌晨2点执行）
echo "0 2 * * 0 /root/maintenance.sh" | crontab -
```

## 🐛 故障排除

### 常见问题

**1. 服务无法启动**
```bash
# 检查端口占用
netstat -tulpn | grep :8080

# 检查依赖库
ldd build/tencent_cloud_cpp_sample

# 查看错误日志
journalctl -u ai3d-server -n 50
```

**2. 数据库连接失败**
```bash
# 测试数据库连接
mysql -h localhost -u ai3d_user -p Tasks

# 检查MySQL服务
sudo systemctl status mysql
```

**3. 编译失败**
```bash
# 清理构建目录
rm -rf build/*
cd build && cmake .. && make -j$(nproc)
```

### 性能问题

**1. 响应慢**
- 检查数据库查询性能
- 优化索引
- 增加连接池大小

**2. 内存不足**
- 调整线程池大小
- 优化内存使用
- 增加服务器内存

**3. 磁盘空间不足**
- 清理日志文件
- 清理临时文件
- 扩展存储空间

## 📋 部署检查清单

### 部署前检查
- [ ] 系统环境满足要求
- [ ] 依赖软件已安装
- [ ] 数据库已配置并测试
- [ ] 配置文件已正确设置
- [ ] 模型存储目录已创建
- [ ] 防火墙规则已配置

### 部署后验证
- [ ] 服务器成功启动
- [ ] 健康检查端点响应正常
- [ ] API接口功能正常
- [ ] 数据库连接正常
- [ ] 文件服务正常
- [ ] 日志记录正常

### 生产环境检查
- [ ] 系统服务已配置
- [ ] 监控告警已设置
- [ ] 日志轮转已配置
- [ ] 备份策略已制定
- [ ] 性能基准测试已通过

## 🎯 总结

本服务器提供了完整的AI3D模型生成服务后端，具有以下特点：

✅ **高性能** - 基于C++和httplib的高性能HTTP服务  
✅ **高可靠** - 完善的错误处理和事务保护  
✅ **高安全** - 多层安全防护机制  
✅ **易部署** - 简单的编译和配置流程  
✅ **易维护** - 完善的日志和监控系统  
✅ **易扩展** - 模块化设计，支持功能扩展  

