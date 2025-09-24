#!/bin/bash

# 测试API更新功能
echo "测试API更新功能..."

# 服务器地址
SERVER="http://localhost:8080"

echo ""
echo "1. 测试 /api/query 接口（新增字段）"
echo "请求: GET $SERVER/api/query?UserId=test_user&PageNum=1&PageSize=5"
curl -s "$SERVER/api/query?UserId=test_user&PageNum=1&PageSize=5" | python3 -m json.tool

echo ""
echo "2. 测试静态文件服务 - 在线预览"
echo "请求: GET $SERVER/model/glb/1704067200000_1234.glb"
curl -I "$SERVER/model/glb/1704067200000_1234.glb"

echo ""
echo "3. 测试下载接口 - 文件下载"
echo "请求: GET $SERVER/download/glb/1704067200000_1234.glb"
curl -I "$SERVER/download/glb/1704067200000_1234.glb"

echo ""
echo "4. 测试预览图片访问"
echo "请求: GET $SERVER/model/glb/1704067200000_1234.jpg"
curl -I "$SERVER/model/glb/1704067200000_1234.jpg"

echo ""
echo "5. 测试下载预览图片"
echo "请求: GET $SERVER/download/glb/1704067200000_1234.jpg"
curl -I "$SERVER/download/glb/1704067200000_1234.jpg"

echo ""
echo "6. 测试 /api/getTaskFiles 接口"
echo "请求: GET $SERVER/api/getTaskFiles?jobId=example_job_123"
curl -s "$SERVER/api/getTaskFiles?jobId=example_job_123" | python3 -m json.tool

echo ""
echo "测试完成！"
echo ""
echo "API端点总结："
echo "- /api/query - 查询任务列表（新增fileurl、previewImages、Isprivate字段）"
echo "- /model/* - 在线预览文件"
echo "- /download/* - 下载文件"
echo "- /api/getTaskFiles - 获取任务文件信息"
