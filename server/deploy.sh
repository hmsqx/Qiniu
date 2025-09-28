#!/bin/bash

# AI3D服务器快速部署脚本
set -e

echo "🚀 AI3D服务器快速部署脚本"
echo "================================"

# 检查是否为root用户
if [ "$EUID" -ne 0 ]; then
    echo "请使用root权限运行此脚本"
    echo "使用方法: sudo ./deploy.sh"
    exit 1
fi

# 检测操作系统
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
else
    echo "无法检测操作系统"
    exit 1
fi

echo "检测到操作系统: $OS $VER"

# 安装依赖
echo "📦 安装系统依赖..."
if [[ "$OS" == *"Ubuntu"* ]] || [[ "$OS" == *"Debian"* ]]; then
    apt update
    apt install -y build-essential cmake libmysqlclient-dev libcurl4-openssl-dev libssl-dev libjsoncpp-dev
elif [[ "$OS" == *"CentOS"* ]] || [[ "$OS" == *"Red Hat"* ]]; then
    yum install -y gcc-c++ cmake mysql-devel libcurl-devel openssl-devel jsoncpp-devel
else
    echo "不支持的操作系统: $OS"
    exit 1
fi

# 检查MySQL
echo "🗄️ 检查MySQL服务..."
if ! systemctl is-active --quiet mysql && ! systemctl is-active --quiet mysqld; then
    echo "MySQL服务未运行，请先安装并启动MySQL"
    echo "Ubuntu/Debian: apt install mysql-server && systemctl start mysql"
    echo "CentOS/RHEL: yum install mysql-server && systemctl start mysqld"
    exit 1
fi

# 创建必要目录
echo "📁 创建必要目录..."
mkdir -p /var/www/models
mkdir -p /var/log/ai3d
chown -R www-data:www-data /var/www/models 2>/dev/null || chown -R nginx:nginx /var/www/models 2>/dev/null || true
chmod -R 755 /var/www/models

# 构建项目
echo "🔨 构建项目..."
chmod +x build.sh
./build.sh

# 创建systemd服务
echo "⚙️ 配置系统服务..."
cat > /etc/systemd/system/ai3d-server.service << EOF
[Unit]
Description=AI3D Server
After=network.target mysql.service

[Service]
Type=simple
User=www-data
WorkingDirectory=$(pwd)/build
ExecStart=$(pwd)/build/tencent_cloud_cpp_sample
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

# 重新加载systemd
systemctl daemon-reload

echo "✅ 部署完成！"
echo ""
echo "📋 后续配置步骤："
echo "1. 编辑配置文件: nano include/config.h"
echo "2. 配置数据库: mysql -u root -p"
echo "3. 导入数据库结构: mysql -u root -p Tasks < database_migration.sql"
echo "4. 启动服务: systemctl start ai3d-server"
echo "5. 设置开机自启: systemctl enable ai3d-server"
echo ""
echo "🔍 服务管理命令："
echo "启动服务: systemctl start ai3d-server"
echo "停止服务: systemctl stop ai3d-server"
echo "重启服务: systemctl restart ai3d-server"
echo "查看状态: systemctl status ai3d-server"
echo "查看日志: journalctl -u ai3d-server -f"
echo ""
echo "🌐 服务地址: http://localhost:8080"
echo "健康检查: curl http://localhost:8080/health"
