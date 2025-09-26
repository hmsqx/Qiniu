#!/bin/bash

# 性能优化测试脚本
echo "=== 服务器性能优化测试 ==="

# 服务器地址
SERVER="http://localhost:8080"

echo ""
echo "=== 1. 基础功能测试 ==="

# 测试用户注册
echo "测试用户注册..."
REGISTER_RESPONSE=$(curl -s -X POST "$SERVER/api/register" \
    -H "Content-Type: application/json" \
    -d '{"username":"testuser","password":"testpass123","email":"test@example.com"}')
echo "注册响应: $REGISTER_RESPONSE"

# 测试用户登录
echo ""
echo "测试用户登录..."
LOGIN_RESPONSE=$(curl -s -X POST "$SERVER/api/login" \
    -H "Content-Type: application/json" \
    -d '{"username":"testuser","password":"testpass123"}')
echo "登录响应: $LOGIN_RESPONSE"

# 提取session token
SESSION_TOKEN=$(echo "$LOGIN_RESPONSE" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data.get('data', {}).get('sessionToken', ''))")

echo ""
echo "=== 2. 并发性能测试 ==="

# 并发请求测试
echo "测试并发请求处理能力..."
echo "发送10个并发请求..."

# 创建临时脚本进行并发测试
cat > /tmp/concurrent_test.sh << 'EOF'
#!/bin/bash
SERVER=$1
TOKEN=$2
for i in {1..10}; do
    (
        START_TIME=$(date +%s%3N)
        RESPONSE=$(curl -s "$SERVER/api/query?UserId=testuser&PageNum=1&PageSize=5" \
            -H "Authorization: Bearer $TOKEN")
        END_TIME=$(date +%s%3N)
        DURATION=$((END_TIME - START_TIME))
        echo "请求 $i: ${DURATION}ms - $(echo $RESPONSE | python3 -c "import sys, json; data=json.load(sys.stdin); print(data.get('status', 'unknown'))")"
    ) &
done
wait
EOF

chmod +x /tmp/concurrent_test.sh
/tmp/concurrent_test.sh "$SERVER" "$SESSION_TOKEN"

echo ""
echo "=== 3. 限流测试 ==="

echo "测试API限流功能..."
echo "快速发送多个请求测试限流..."

# 快速发送请求测试限流
for i in {1..15}; do
    RESPONSE=$(curl -s "$SERVER/api/query?UserId=testuser&PageNum=1&PageSize=5" \
        -H "Authorization: Bearer $SESSION_TOKEN")
    STATUS=$(echo "$RESPONSE" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data.get('status', 'unknown'))" 2>/dev/null)
    HTTP_STATUS=$(curl -s -o /dev/null -w "%{http_code}" "$SERVER/api/query?UserId=testuser&PageNum=1&PageSize=5" \
        -H "Authorization: Bearer $SESSION_TOKEN")
    
    if [ "$HTTP_STATUS" = "429" ]; then
        echo "请求 $i: 限流生效 (HTTP $HTTP_STATUS)"
        break
    else
        echo "请求 $i: 正常 (HTTP $HTTP_STATUS)"
    fi
    
    sleep 0.1
done

echo ""
echo "=== 4. 文件下载测试 ==="

echo "测试文件访问功能..."

# 测试静态文件访问
echo "测试模型文件访问..."
MODEL_RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" "$SERVER/model/glb/test.glb")
echo "模型文件访问状态: $MODEL_RESPONSE"

# 测试下载接口
echo "测试下载接口..."
DOWNLOAD_RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" "$SERVER/download/glb/test.glb")
echo "下载接口状态: $DOWNLOAD_RESPONSE"

echo ""
echo "=== 5. 错误处理测试 ==="

echo "测试错误处理..."

# 测试无效参数
echo "测试无效参数..."
INVALID_RESPONSE=$(curl -s "$SERVER/api/query")
echo "无效参数响应: $(echo "$INVALID_RESPONSE" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data.get('message', 'unknown'))" 2>/dev/null)"

# 测试无效token
echo "测试无效token..."
INVALID_TOKEN_RESPONSE=$(curl -s "$SERVER/api/query?UserId=testuser&PageNum=1&PageSize=5" \
    -H "Authorization: Bearer invalid_token")
echo "无效token响应: $(echo "$INVALID_TOKEN_RESPONSE" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data.get('message', 'unknown'))" 2>/dev/null)"

echo ""
echo "=== 6. 性能监控测试 ==="

echo "测试性能监控功能..."

# 发送一些请求以产生监控数据
for i in {1..5}; do
    curl -s "$SERVER/api/query?UserId=testuser&PageNum=1&PageSize=5" \
        -H "Authorization: Bearer $SESSION_TOKEN" > /dev/null
done

echo "已发送5个请求用于性能监控测试"

echo ""
echo "=== 7. 数据库连接池测试 ==="

echo "测试数据库连接池..."

# 测试数据库操作的响应时间
echo "测试数据库查询性能..."
START_TIME=$(date +%s%3N)
DB_RESPONSE=$(curl -s "$SERVER/api/query?UserId=testuser&PageNum=1&PageSize=5" \
    -H "Authorization: Bearer $SESSION_TOKEN")
END_TIME=$(date +%s%3N)
DB_DURATION=$((END_TIME - START_TIME))
echo "数据库查询耗时: ${DB_DURATION}ms"

echo ""
echo "=== 8. 安全测试 ==="

echo "测试安全功能..."

# 测试SQL注入防护
echo "测试SQL注入防护..."
SQL_INJECTION_RESPONSE=$(curl -s "$SERVER/api/query?UserId=testuser'; DROP TABLE users; --&PageNum=1&PageSize=5" \
    -H "Authorization: Bearer $SESSION_TOKEN")
echo "SQL注入测试响应状态: $(echo "$SQL_INJECTION_RESPONSE" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data.get('status', 'unknown'))" 2>/dev/null)"

# 测试XSS防护
echo "测试XSS防护..."
XSS_RESPONSE=$(curl -s "$SERVER/api/query?UserId=<script>alert('xss')</script>&PageNum=1&PageSize=5" \
    -H "Authorization: Bearer $SESSION_TOKEN")
echo "XSS测试响应状态: $(echo "$XSS_RESPONSE" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data.get('status', 'unknown'))" 2>/dev/null)"

echo ""
echo "=== 测试总结 ==="

echo "✅ 基础功能测试完成"
echo "✅ 并发性能测试完成"
echo "✅ 限流功能测试完成"
echo "✅ 文件下载测试完成"
echo "✅ 错误处理测试完成"
echo "✅ 性能监控测试完成"
echo "✅ 数据库连接池测试完成"
echo "✅ 安全功能测试完成"

echo ""
echo "=== 性能优化建议 ==="
echo "1. 监控服务器日志查看性能指标"
echo "2. 检查数据库连接池状态"
echo "3. 观察内存和CPU使用情况"
echo "4. 测试高并发场景下的稳定性"
echo "5. 定期检查错误日志和异常报告"

echo ""
echo "=== 清理临时文件 ==="
rm -f /tmp/concurrent_test.sh

echo ""
echo "性能优化测试完成！"
echo ""
echo "如需查看详细性能报告，请检查："
echo "- 服务器日志输出"
echo "- 性能监控数据"
echo "- 数据库连接池状态"
echo "- 系统资源使用情况"
