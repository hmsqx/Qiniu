#!/bin/bash

# 测试新的文件下载结构
echo "测试新的文件下载结构..."

# 创建测试目录结构
echo "创建测试目录..."
mkdir -p /root/Qiniu/server/model/{obj,fbx,glb,stl,usdz,mp4}

# 创建测试文件
echo "创建测试文件..."

# GLB格式测试
echo "测试GLB格式..."
echo "GLB模型内容" > /root/Qiniu/server/model/glb/1704067200000_1234.glb
echo "GLB预览图片内容" > /root/Qiniu/server/model/glb/1704067200000_1234.jpg

# OBJ格式测试
echo "测试OBJ格式..."
echo "OBJ模型内容" > /root/Qiniu/server/model/obj/1704067200001_5678.obj
echo "OBJ预览图片内容" > /root/Qiniu/server/model/obj/1704067200001_5678.jpg

# FBX格式测试
echo "测试FBX格式..."
echo "FBX模型内容" > /root/Qiniu/server/model/fbx/1704067200002_9012.fbx
echo "FBX预览图片内容" > /root/Qiniu/server/model/fbx/1704067200002_9012.jpg

echo "测试文件创建完成！"

# 验证文件结构
echo ""
echo "验证文件结构："
echo "GLB目录内容："
ls -la /root/Qiniu/server/model/glb/

echo ""
echo "OBJ目录内容："
ls -la /root/Qiniu/server/model/obj/

echo ""
echo "FBX目录内容："
ls -la /root/Qiniu/server/model/fbx/

echo ""
echo "测试完成！新的文件结构已准备就绪。"
echo ""
echo "可以通过以下URL访问文件："
echo "模型文件："
echo "  http://localhost:8080/model/glb/1704067200000_1234.glb"
echo "  http://localhost:8080/model/obj/1704067200001_5678.obj"
echo "  http://localhost:8080/model/fbx/1704067200002_9012.fbx"
echo ""
echo "预览图片："
echo "  http://localhost:8080/model/glb/1704067200000_1234.jpg"
echo "  http://localhost:8080/model/obj/1704067200001_5678.jpg"
echo "  http://localhost:8080/model/fbx/1704067200002_9012.jpg"
