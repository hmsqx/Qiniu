# 文件结构优化更新

## 更新概述

根据您的要求，我们已将文件存储结构进行了优化，现在预览图片和模型文件放在同一目录下，使用相同的文件名但不同的后缀。

## 主要变更

### 1. 文件存储结构变更

**之前的结构：**
```
/model/
├── obj/
├── fbx/
├── glb/
├── stl/
├── usdz/
├── mp4/
└── preview/    # 单独的预览图片目录
```

**现在的结构：**
```
/model/
├── obj/        # OBJ模型文件和预览图片
├── fbx/        # FBX模型文件和预览图片
├── glb/        # GLB模型文件和预览图片
├── stl/        # STL模型文件和预览图片
├── usdz/       # USDZ模型文件和预览图片
└── mp4/        # MP4视频文件和预览图片
```

### 2. 文件命名规则

现在同一模型的模型文件和预览图片使用相同的基础文件名：

- **模型文件**：`时间戳_随机数.格式扩展名`
- **预览图片**：`时间戳_随机数.jpg`

**示例：**
- `/model/glb/1704067200000_1234.glb` - GLB模型文件
- `/model/glb/1704067200000_1234.jpg` - 对应的预览图片

### 3. 代码修改

#### 修改的文件：
1. **`src/model_downloader.cc`**
   - 更新了 `downloadPreviewImage` 函数，现在接受基础文件名参数
   - 修改了 `downloadModelFiles` 函数，确保预览图片与模型文件在同一目录

2. **`include/model_downloader.h`**
   - 更新了函数声明

3. **`database_migration.sql`**
   - 更新了注释说明新的文件结构

#### 删除的内容：
- 删除了 `/model/preview/` 目录
- 移除了相关的preview目录引用

### 4. 静态文件服务

静态文件服务 (`main.cc` 中的 `/model/.*` 路由) 已经支持新的文件结构，可以正确处理：

- 模型文件：`.obj`, `.fbx`, `.glb`, `.stl`, `.usdz`, `.mp4`
- 预览图片：`.jpg`, `.jpeg`, `.png`

### 5. 数据库存储

数据库中的字段保持不变：
- `fileurl` - 存储模型文件的本地URL
- `previewImages` - 存储预览图片的本地URL

**示例数据：**
```
fileurl: "/model/glb/1704067200000_1234.glb,/model/obj/1704067200001_5678.obj"
previewImages: "/model/glb/1704067200000_1234.jpg,/model/obj/1704067200001_5678.jpg"
```

## 优势

1. **文件组织更清晰**：相关文件放在同一目录下，便于管理
2. **命名更一致**：模型文件和预览图片使用相同的基础文件名
3. **访问更简单**：可以通过文件名轻松找到对应的预览图片
4. **维护更容易**：删除模型时，对应的预览图片也在同一目录下
5. **路径更简洁**：不需要单独的preview目录

## 测试验证

已创建测试脚本 `test_download_structure.sh` 来验证新的文件结构：

```bash
./test_download_structure.sh
```

测试结果确认：
- 文件可以正确创建在同一目录下
- 文件名遵循新的命名规则
- 静态文件服务可以正确访问文件

## HTTP访问示例

### 模型文件访问
```
http://localhost:8080/model/glb/1704067200000_1234.glb
http://localhost:8080/model/obj/1704067200001_5678.obj
http://localhost:8080/model/fbx/1704067200002_9012.fbx
```

### 预览图片访问
```
http://localhost:8080/model/glb/1704067200000_1234.jpg
http://localhost:8080/model/obj/1704067200001_5678.jpg
http://localhost:8080/model/fbx/1704067200002_9012.jpg
```

## 注意事项

1. **向后兼容性**：现有的API端点保持不变
2. **文件权限**：确保所有格式目录存在且有写权限
3. **预览图片格式**：统一使用.jpg格式
4. **文件名唯一性**：使用时间戳+随机数确保文件名不重复

## 部署说明

1. 确保所有格式目录存在：
   ```bash
   mkdir -p /root/Qiniu/server/model/{obj,fbx,glb,stl,usdz,mp4}
   ```

2. 运行数据库迁移（如果尚未执行）：
   ```bash
   mysql -u Qiniu -p Tasks < database_migration.sql
   ```

3. 重新编译和启动服务：
   ```bash
   cd /root/Qiniu/server
   mkdir -p build && cd build
   cmake .. && make
   ./tencent_cloud_cpp_sample
   ```

现在您的系统已经按照要求优化了文件存储结构，预览图片和模型文件将放在同一目录下，使用相同的文件名但不同的后缀。
