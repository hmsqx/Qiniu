#!/bin/bash

# 测试重复下载修复功能
echo "测试重复下载修复功能..."

# 服务器地址
SERVER="http://localhost:8080"

echo ""
echo "=== 测试场景说明 ==="
echo "1. 第一次查询任务 - 应该下载文件到本地"
echo "2. 第二次查询相同任务 - 应该跳过下载，使用现有文件"
echo ""

# 测试任务ID（请替换为实际的任务ID）
TEST_JOB_ID="test_job_123"
TEST_USER_ID="test_user"

echo "测试任务ID: $TEST_JOB_ID"
echo "测试用户ID: $TEST_USER_ID"

echo ""
echo "=== 第一次查询任务（应该触发下载） ==="
echo "请求: GET $SERVER/api/query?UserId=$TEST_USER_ID&PageNum=1&PageSize=5"
FIRST_RESPONSE=$(curl -s "$SERVER/api/query?UserId=$TEST_USER_ID&PageNum=1&PageSize=5")
echo "响应:"
echo "$FIRST_RESPONSE" | python3 -m json.tool

echo ""
echo "=== 等待2秒后再次查询（应该跳过下载） ==="
sleep 2

echo ""
echo "=== 第二次查询相同任务（应该跳过下载） ==="
echo "请求: GET $SERVER/api/query?UserId=$TEST_USER_ID&PageNum=1&PageSize=5"
SECOND_RESPONSE=$(curl -s "$SERVER/api/query?UserId=$TEST_USER_ID&PageNum=1&PageSize=5")
echo "响应:"
echo "$SECOND_RESPONSE" | python3 -m json.tool

echo ""
echo "=== 验证服务器日志 ==="
echo "请检查服务器日志，应该看到："
echo "- 第一次查询：'任务 XXX 不存在本地文件，开始下载' 或 '任务 XXX 不存在数据库记录，开始下载'"
echo "- 第二次查询：'任务 XXX 已存在本地文件，跳过下载'"

echo ""
echo "=== 测试文件访问 ==="
echo "如果任务已完成，可以测试文件访问："
echo "GET $SERVER/api/getTaskFiles?jobId=$TEST_JOB_ID"
curl -s "$SERVER/api/getTaskFiles?jobId=$TEST_JOB_ID" | python3 -m json.tool

echo ""
echo "测试完成！"
echo ""
echo "=== 修复说明 ==="
echo "现在系统会："
echo "1. 检查数据库中是否已存在本地文件URL"
echo "2. 如果存在，直接使用现有文件，不重新下载"
echo "3. 如果不存在，才进行下载并保存到数据库"
echo "4. 避免重复下载和文件重复问题"
