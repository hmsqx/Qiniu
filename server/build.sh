#!/bin/bash

# AI3D服务器构建脚本
set -e

echo "开始构建AI3D服务器..."

# 检查依赖
echo "检查构建依赖..."
if ! command -v cmake &> /dev/null; then
    echo "错误: cmake 未安装"
    exit 1
fi

if ! command -v make &> /dev/null; then
    echo "错误: make 未安装"
    exit 1
fi

# 创建构建目录
echo "创建构建目录..."
mkdir -p build
cd build

# 配置项目
echo "配置项目..."
if command -v cmake3 &> /dev/null; then
    # CentOS系统使用cmake3
    cmake3 ..
else
    cmake ..
fi

# 编译项目
echo "编译项目..."
make -j$(nproc)

echo "构建完成！"
echo "可执行文件位置: $(pwd)/tencent_cloud_cpp_sample"

# 检查可执行文件
if [ -f "tencent_cloud_cpp_sample" ]; then
    echo "✅ 构建成功"
    echo "启动命令: ./tencent_cloud_cpp_sample"
else
    echo "❌ 构建失败"
    exit 1
fi