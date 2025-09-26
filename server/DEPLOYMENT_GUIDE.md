# 服务器部署指南

## 🚀 快速部署

### 1. 环境要求

**系统要求：**
- Linux (Ubuntu 18.04+ 推荐)
- 内存：至少 2GB RAM
- 存储：至少 10GB 可用空间
- CPU：2核心以上

**依赖软件：**
- CMake 3.1+
- GCC 7.0+ (支持C++17)
- MySQL 5.7+ 或 MariaDB 10.3+
- libcurl
- OpenSSL

### 2. 安装依赖

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y build-essential cmake libmysqlclient-dev libcurl4-openssl-dev libssl-dev

# CentOS/RHEL
sudo yum install -y gcc-c++ cmake mysql-devel libcurl-devel openssl-devel
```

### 3. 数据库配置

```sql
-- 创建数据库
CREATE DATABASE ai3d_server CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 创建用户
CREATE USER 'ai3d_user'@'localhost' IDENTIFIED BY 'your_password';
GRANT ALL PRIVILEGES ON ai3d_server.* TO 'ai3d_user'@'localhost';
FLUSH PRIVILEGES;

-- 导入数据库结构
mysql -u ai3d_user -p ai3d_server < database_migration.sql
```

### 4. 配置文件

创建 `config.h` 文件：

```cpp
#pragma once

// 腾讯云配置
#define TENCENTCLOUD_SECRET_ID "your_secret_id"
#define TENCENTCLOUD_SECRET_KEY "your_secret_key"

// 数据库配置
#define MYSQL_HOST "localhost"
#define MYSQL_PORT 3306
#define MYSQL_USER "ai3d_user"
#define MYSQL_PASSWORD "your_password"
#define MYSQL_DATABASE "ai3d_server"
```

### 5. 编译服务器

```bash
cd /root/Qiniu/server
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

### 6. 启动服务器

```bash
# 前台运行（调试模式）
./tencent_cloud_cpp_sample

# 后台运行（生产模式）
nohup ./tencent_cloud_cpp_sample > server.log 2>&1 &
```

## 🔧 配置选项

### 环境配置

根据不同环境选择合适的配置：

```cpp
// 开发环境
ServerInitializer::initialize(ServerInitializer::getDevelopmentConfig());

// 生产环境（默认）
ServerInitializer::initialize(ServerInitializer::getDefaultConfig());

// 高并发环境
ServerInitializer::initialize(ServerInitializer::getHighConcurrencyConfig());
```

### 自定义配置

```cpp
ServerConfig customConfig;
customConfig.threadPoolSize = 16;
customConfig.dbConfig.maxConnections = 30;
customConfig.securityConfig.maxRequestsPerMinute = 100;
customConfig.maxConcurrentDownloads = 8;

ServerInitializer::initialize(customConfig);
```

## 📊 性能调优

### 1. 数据库优化

```sql
-- 添加索引
CREATE INDEX idx_ai3d_tasks_user_id ON ai3d_tasks(user_id);
CREATE INDEX idx_ai3d_tasks_status ON ai3d_tasks(status);
CREATE INDEX idx_ai3d_tasks_create_time ON ai3d_tasks(create_time);

-- 优化MySQL配置
[mysqld]
innodb_buffer_pool_size = 1G
innodb_log_file_size = 256M
max_connections = 200
query_cache_size = 128M
```

### 2. 系统优化

```bash
# 增加文件描述符限制
echo "* soft nofile 65535" >> /etc/security/limits.conf
echo "* hard nofile 65535" >> /etc/security/limits.conf

# 优化网络参数
echo "net.core.somaxconn = 65535" >> /etc/sysctl.conf
echo "net.ipv4.tcp_max_syn_backlog = 65535" >> /etc/sysctl.conf
sysctl -p
```

### 3. 服务器配置

**小型部署（<1000用户）：**
- 线程池：4-8个线程
- 数据库连接池：5-10个连接
- 限流：60 req/min per IP

**中型部署（1000-10000用户）：**
- 线程池：8-16个线程
- 数据库连接池：10-20个连接
- 限流：100 req/min per IP

**大型部署（>10000用户）：**
- 线程池：16-32个线程
- 数据库连接池：20-50个连接
- 限流：200 req/min per IP

## 🔒 安全配置

### 1. 防火墙设置

```bash
# 只允许必要的端口
sudo ufw allow 8080/tcp  # HTTP服务端口
sudo ufw allow 22/tcp    # SSH端口
sudo ufw enable
```

### 2. SSL/TLS配置

```cpp
// 在main.cc中添加SSL支持
server.set_mount_point("/", "./static/");
server.set_file_extension_and_mimetype_mapping("pem", "application/x-pem-file");
server.set_file_extension_and_mimetype_mapping("key", "application/x-pem-file");

// 启用HTTPS
if (server.listen("0.0.0.0", 8443, "server.pem", "server.key")) {
    std::cout << "HTTPS服务器启动成功，端口: 8443" << std::endl;
}
```

### 3. 访问控制

```cpp
// 在security_middleware.h中配置白名单
std::vector<std::string> allowedIPs = {
    "192.168.1.0/24",
    "10.0.0.0/8"
};
```

## 📈 监控和日志

### 1. 日志配置

```cpp
// 添加日志系统
#include <fstream>

class Logger {
public:
    static void log(const std::string& message) {
        std::ofstream logFile("/var/log/ai3d_server.log", std::ios::app);
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        logFile << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
                << " - " << message << std::endl;
    }
};
```

### 2. 性能监控

```bash
# 使用htop监控系统资源
htop

# 监控网络连接
netstat -tulpn | grep :8080

# 监控数据库连接
mysql -u root -p -e "SHOW PROCESSLIST;"
```

### 3. 健康检查

```bash
# 检查服务器状态
curl http://localhost:8080/health

# 检查API响应时间
curl -w "@curl-format.txt" -o /dev/null -s http://localhost:8080/api/query
```

创建 `curl-format.txt`：
```
     time_namelookup:  %{time_namelookup}\n
        time_connect:  %{time_connect}\n
     time_appconnect:  %{time_appconnect}\n
    time_pretransfer:  %{time_pretransfer}\n
       time_redirect:  %{time_redirect}\n
  time_starttransfer:  %{time_starttransfer}\n
                     ----------\n
          time_total:  %{time_total}\n
```

## 🔄 更新和维护

### 1. 热更新

```bash
# 优雅重启
pkill -USR1 tencent_cloud_cpp_sample

# 零停机更新
# 1. 编译新版本
make -j$(nproc)

# 2. 备份当前版本
cp tencent_cloud_cpp_sample tencent_cloud_cpp_sample.backup

# 3. 替换新版本
cp tencent_cloud_cpp_sample.new tencent_cloud_cpp_sample

# 4. 重启服务
systemctl restart ai3d-server
```

### 2. 定期维护

```bash
# 创建维护脚本
cat > /root/maintenance.sh << 'EOF'
#!/bin/bash

# 清理日志文件
find /var/log -name "*.log" -mtime +30 -delete

# 清理临时文件
find /tmp -name "ai3d_*" -mtime +7 -delete

# 数据库维护
mysql -u root -p -e "OPTIMIZE TABLE ai3d_tasks;"

# 重启服务（如果需要）
# systemctl restart ai3d-server

echo "维护任务完成: $(date)"
EOF

chmod +x /root/maintenance.sh

# 添加到crontab
echo "0 2 * * 0 /root/maintenance.sh" | crontab -
```

### 3. 备份策略

```bash
# 数据库备份
cat > /root/backup_db.sh << 'EOF'
#!/bin/bash
DATE=$(date +%Y%m%d_%H%M%S)
mysqldump -u root -p ai3d_server > /backup/ai3d_server_$DATE.sql
gzip /backup/ai3d_server_$DATE.sql
find /backup -name "*.gz" -mtime +30 -delete
EOF

chmod +x /root/backup_db.sh

# 每日备份
echo "0 1 * * * /root/backup_db.sh" | crontab -
```

## 🚨 故障排除

### 1. 常见问题

**服务器无法启动：**
```bash
# 检查端口占用
netstat -tulpn | grep :8080

# 检查依赖库
ldd tencent_cloud_cpp_sample

# 查看错误日志
tail -f /var/log/ai3d_server.log
```

**数据库连接失败：**
```bash
# 测试数据库连接
mysql -h localhost -u ai3d_user -p ai3d_server

# 检查MySQL服务状态
systemctl status mysql
```

**性能问题：**
```bash
# 检查系统资源
top
free -h
df -h

# 检查数据库性能
mysql -u root -p -e "SHOW PROCESSLIST;"
```

### 2. 日志分析

```bash
# 分析错误日志
grep "ERROR" /var/log/ai3d_server.log | tail -20

# 分析响应时间
grep "response_time" /var/log/ai3d_server.log | awk '{print $NF}' | sort -n

# 分析请求统计
grep "GET\|POST" /var/log/ai3d_server.log | awk '{print $6}' | sort | uniq -c
```

### 3. 紧急恢复

```bash
# 快速重启服务
systemctl restart ai3d-server

# 回滚到备份版本
cp tencent_cloud_cpp_sample.backup tencent_cloud_cpp_sample
systemctl restart ai3d-server

# 恢复数据库
gunzip -c /backup/ai3d_server_20240101_120000.sql.gz | mysql -u root -p ai3d_server
```

## 📋 部署检查清单

### 部署前检查

- [ ] 系统环境满足要求
- [ ] 依赖软件已安装
- [ ] 数据库已配置并测试
- [ ] 配置文件已正确设置
- [ ] 防火墙规则已配置
- [ ] SSL证书已准备（如需要）

### 部署后验证

- [ ] 服务器成功启动
- [ ] 健康检查端点响应正常
- [ ] API接口功能正常
- [ ] 数据库连接正常
- [ ] 性能监控工作正常
- [ ] 日志记录正常
- [ ] 安全功能生效

### 生产环境检查

- [ ] 备份策略已配置
- [ ] 监控告警已设置
- [ ] 日志轮转已配置
- [ ] 定期维护任务已安排
- [ ] 灾难恢复计划已制定
- [ ] 性能基准测试已通过

## 🎯 总结

本部署指南提供了完整的服务器部署、配置、监控和维护方案。通过遵循这些步骤，您可以：

✅ **快速部署** - 一键式部署流程  
✅ **性能优化** - 针对不同规模的配置  
✅ **安全防护** - 多层安全机制  
✅ **监控告警** - 实时性能监控  
✅ **故障恢复** - 完善的备份和恢复策略  
✅ **持续维护** - 自动化维护任务  

确保在生产环境中严格按照本指南操作，并根据实际需求调整配置参数。
