#!/bin/bash

echo "停止文本和图像优化处理服务..."

# 查找运行的 python -m app.main 进程
PIDS=$(ps aux | grep "python -m app.main" | grep -v grep | awk '{print $2}')

if [ -z "$PIDS" ]; then
  echo "未找到正在运行的服务。"
else
  echo "找到服务进程: $PIDS"
  kill -9 $PIDS
  echo "服务已终止。"
fi