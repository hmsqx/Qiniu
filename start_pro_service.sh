#!/bin/bash

echo "启动文本和图像优化处理服务..."

# 使用 nohup 在后台运行，输出重定向到日志文件
# python -m app.main 
nohup python -m app.main > logs/pro_service.log 2>&1 &

echo "服务已在后台启动"
echo "日志: logs/pro_service.log"