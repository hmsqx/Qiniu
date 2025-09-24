# 3D模型下载功能说明

## 功能概述

本功能实现了从腾讯云AI3D服务生成的3D模型自动下载到本地服务器的功能，支持多种3D模型格式，并将文件按格式分类存储。

## 支持的文件格式

- **OBJ** - 3D模型文件
- **FBX** - 3D模型文件  
- **GLB** - 3D模型文件
- **STL** - 3D模型文件
- **USDZ** - 3D模型文件
- **MP4** - 视频文件
- **JPG/PNG** - 预览图片

## 文件存储结构

```
/root/Qiniu/server/model/
├── obj/          # OBJ格式模型文件和预览图片
├── fbx/          # FBX格式模型文件和预览图片
├── glb/          # GLB格式模型文件和预览图片
├── stl/          # STL格式模型文件和预览图片
├── usdz/         # USDZ格式模型文件和预览图片
└── mp4/          # MP4视频文件和预览图片
```

### 文件命名规则

预览图片和模型文件放在同一目录下，使用相同的文件名但不同的后缀：

- 模型文件：`时间戳_随机数.格式扩展名`
- 预览图片：`时间戳_随机数.jpg`

例如：
- `/model/glb/1704067200000_1234.glb` - GLB模型文件
- `/model/glb/1704067200000_1234.jpg` - 对应的预览图片

## 工作流程

1. **模型生成完成**: 当腾讯云AI3D任务状态变为"DONE"时
2. **自动下载**: 系统自动从腾讯云下载模型文件和预览图片
3. **格式分类**: 根据文件格式将文件保存到对应的子目录
4. **随机命名**: 使用时间戳+随机数生成唯一文件名，避免重复
5. **数据库更新**: 将本地文件URL保存到ai3d_tasks表的fileurl和previewImages字段
6. **HTTP服务**: 通过HTTP服务器提供文件访问服务

## 数据库变更

### 新增字段

```sql
-- 添加fileurl字段，存储模型文件的本地URL
ALTER TABLE ai3d_tasks ADD COLUMN fileurl TEXT COMMENT '本地模型文件URL，多个文件用逗号分隔';

-- 添加previewImages字段，存储预览图片的本地URL  
ALTER TABLE ai3d_tasks ADD COLUMN previewImages TEXT COMMENT '本地预览图片URL，多个图片用逗号分隔';
```

### 数据库迁移

运行以下SQL脚本进行数据库迁移：

```bash
mysql -u Qiniu -p Tasks < database_migration.sql
```

## 文件命名规则

文件名格式：`时间戳_随机数.扩展名`

例如：`1704067200000_1234.obj`

- 时间戳：毫秒级时间戳，确保时间唯一性
- 随机数：4位随机数，避免并发冲突
- 扩展名：根据文件格式确定

## HTTP访问

下载的模型文件可以通过两种方式访问：

### 1. 在线预览（/model/*）

用于在网页中直接显示文件内容：

- **模型文件**: `http://服务器地址:8080/model/格式/文件名.扩展名`
- **预览图片**: `http://服务器地址:8080/model/格式/文件名.jpg`

### 2. 文件下载（/download/*）

用于触发浏览器下载文件：

- **模型文件**: `http://服务器地址:8080/download/格式/文件名.扩展名`
- **预览图片**: `http://服务器地址:8080/download/格式/文件名.jpg`

### 示例URL

#### 在线预览
```
模型文件：
http://localhost:8080/model/glb/1704067200000_1234.glb
http://localhost:8080/model/obj/1704067200001_5678.obj

预览图片：
http://localhost:8080/model/glb/1704067200000_1234.jpg
http://localhost:8080/model/obj/1704067200001_5678.jpg
```

#### 文件下载
```
模型文件：
http://localhost:8080/download/glb/1704067200000_1234.glb
http://localhost:8080/download/obj/1704067200001_5678.obj

预览图片：
http://localhost:8080/download/glb/1704067200000_1234.jpg
http://localhost:8080/download/obj/1704067200001_5678.jpg
```

## 技术实现

### 核心文件

- `include/model_downloader.h` - 模型下载器头文件
- `src/model_downloader.cc` - 模型下载器实现
- `src/tx_ai3d.cc` - 集成下载功能到AI3D任务处理
- `src/db_utils.cc` - 数据库操作函数
- `main.cc` - HTTP静态文件服务

### 依赖库

- **libcurl** - HTTP文件下载
- **jsoncpp** - JSON数据处理
- **mysqlclient** - MySQL数据库操作
- **filesystem** - 文件系统操作

### 编译配置

CMakeLists.txt已更新，添加了CURL库依赖：

```cmake
find_package(CURL REQUIRED)
target_link_libraries(tencent_cloud_cpp_sample 
    # ... 其他库
    CURL::libcurl
)
```

## 错误处理

1. **下载失败**: 如果文件下载失败，系统会记录错误日志，但不影响原有功能
2. **文件重复**: 使用时间戳+随机数确保文件名唯一性
3. **网络超时**: 设置5分钟下载超时，避免长时间等待
4. **存储空间**: 定期清理过期文件，避免磁盘空间不足

## 安全考虑

1. **路径验证**: 静态文件服务验证文件路径，防止目录遍历攻击
2. **文件类型限制**: 只允许访问预定义的文件格式
3. **访问控制**: 可扩展添加用户权限验证

## 监控和日志

系统会输出详细的下载日志：

```
成功下载文件: /root/Qiniu/server/model/obj/1704067200000_1234.obj (大小: 1024000 字节)
下载模型文件失败: https://example.com/model.obj
```

## 性能优化

1. **并发下载**: 支持多个文件同时下载
2. **缓存控制**: HTTP响应设置适当的缓存头
3. **文件压缩**: 可根据需要添加文件压缩功能

## 使用示例

当AI3D任务完成时，系统会自动：

1. 查询任务状态获取文件URL
2. 下载文件到本地服务器
3. 更新数据库记录
4. 返回本地文件URL给客户端

客户端可以通过返回的本地URL直接访问模型文件，无需再次从腾讯云下载。
