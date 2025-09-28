#!/bin/bash

# AI3D服务器健康检查脚本
set -e

# 配置
SERVER_URL="http://localhost:8080"
HEALTH_ENDPOINT="/health"
API_ENDPOINT="/api/query"
LOG_FILE="/var/log/ai3d/health_check.log"

# 创建日志目录
mkdir -p /var/log/ai3d

# 日志函数
log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

# 检查服务是否运行
check_service_running() {
    if systemctl is-active --quiet ai3d-server; then
        log "✅ 服务正在运行"
        return 0
    else
        log "❌ 服务未运行"
        return 1
    fi
}

# 检查端口是否监听
check_port_listening() {
    if netstat -tuln | grep -q ":8080 "; then
        log "✅ 端口8080正在监听"
        return 0
    else
        log "❌ 端口8080未监听"
        return 1
    fi
}

# 检查健康端点
check_health_endpoint() {
    if curl -f -s "$SERVER_URL$HEALTH_ENDPOINT" > /dev/null; then
        log "✅ 健康检查端点响应正常"
        return 0
    else
        log "❌ 健康检查端点无响应"
        return 1
    fi
}

# 检查API端点
check_api_endpoint() {
    local response=$(curl -s -w "%{http_code}" -o /dev/null "$SERVER_URL$API_ENDPOINT")
    if [ "$response" = "200" ] || [ "$response" = "401" ]; then
        log "✅ API端点响应正常 (HTTP $response)"
        return 0
    else
        log "❌ API端点响应异常 (HTTP $response)"
        return 1
    fi
}

# 检查数据库连接
check_database_connection() {
    if mysql -h localhost -u root -e "SELECT 1;" > /dev/null 2>&1; then
        log "✅ 数据库连接正常"
        return 0
    else
        log "❌ 数据库连接失败"
        return 1
    fi
}

# 检查磁盘空间
check_disk_space() {
    local usage=$(df /var/www/models | awk 'NR==2 {print $5}' | sed 's/%//')
    if [ "$usage" -lt 90 ]; then
        log "✅ 磁盘空间充足 ($usage% 已使用)"
        return 0
    else
        log "⚠️ 磁盘空间不足 ($usage% 已使用)"
        return 1
    fi
}

# 检查内存使用
check_memory_usage() {
    local usage=$(free | awk 'NR==2{printf "%.0f", $3*100/$2}')
    if [ "$usage" -lt 90 ]; then
        log "✅ 内存使用正常 ($usage% 已使用)"
        return 0
    else
        log "⚠️ 内存使用过高 ($usage% 已使用)"
        return 1
    fi
}

# 重启服务
restart_service() {
    log "🔄 尝试重启服务..."
    systemctl restart ai3d-server
    sleep 10
    
    if check_service_running; then
        log "✅ 服务重启成功"
        return 0
    else
        log "❌ 服务重启失败"
        return 1
    fi
}

# 发送告警
send_alert() {
    local message="$1"
    log "🚨 告警: $message"
    
    # 这里可以添加发送邮件、短信等告警逻辑
    # 例如: echo "$message" | mail -s "AI3D服务器告警" admin@example.com
}

# 主检查函数
main() {
    log "开始健康检查..."
    
    local failed_checks=0
    
    # 基础检查
    if ! check_service_running; then
        ((failed_checks++))
    fi
    
    if ! check_port_listening; then
        ((failed_checks++))
    fi
    
    # 服务检查
    if ! check_health_endpoint; then
        ((failed_checks++))
    fi
    
    if ! check_api_endpoint; then
        ((failed_checks++))
    fi
    
    # 系统检查
    if ! check_database_connection; then
        ((failed_checks++))
    fi
    
    if ! check_disk_space; then
        ((failed_checks++))
    fi
    
    if ! check_memory_usage; then
        ((failed_checks++))
    fi
    
    # 结果处理
    if [ "$failed_checks" -eq 0 ]; then
        log "✅ 所有检查通过，服务运行正常"
        exit 0
    elif [ "$failed_checks" -le 2 ]; then
        log "⚠️ 发现 $failed_checks 个问题，尝试自动修复..."
        
        # 尝试重启服务
        if ! check_health_endpoint; then
            restart_service
        fi
    else
        log "❌ 发现 $failed_checks 个严重问题，需要人工干预"
        send_alert "AI3D服务器发现 $failed_checks 个严重问题"
        exit 1
    fi
}

# 显示帮助信息
show_help() {
    echo "AI3D服务器健康检查脚本"
    echo ""
    echo "使用方法:"
    echo "  $0          # 执行完整健康检查"
    echo "  $0 --help   # 显示帮助信息"
    echo ""
    echo "检查项目:"
    echo "  - 服务运行状态"
    echo "  - 端口监听状态"
    echo "  - 健康检查端点"
    echo "  - API端点响应"
    echo "  - 数据库连接"
    echo "  - 磁盘空间"
    echo "  - 内存使用"
    echo ""
    echo "日志文件: $LOG_FILE"
}

# 处理命令行参数
case "${1:-}" in
    --help|-h)
        show_help
        exit 0
        ;;
    *)
        main
        ;;
esac
