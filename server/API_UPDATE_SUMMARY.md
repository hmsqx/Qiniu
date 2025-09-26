## 管理员端新增接口

本次新增了管理员统计与查询接口（需 `Session-Token` 对应用户 `role=admin`）。

### 1) 管理员概览 `/api/admin/overview` [GET]
- 功能：一次返回四类数据
  - downloadedModels / totalModels → 下载率（被至少下载过一次的模型比例）
  - likedModels / totalModels → 点赞率（被至少点赞过一次的模型比例）
  - totalUsers → 当前用户总数
  - userGrowthRate → 用户增长率（昨日新增 vs 前日新增）
- 响应：
  - data.totalModels, data.downloadedModels, data.likedModels
  - data.downloadRate, data.likeRate
  - data.totalUsers
  - data.yesterdayNewUsers, data.dayBeforeNewUsers, data.userGrowthRate

### 2) 管理员用户查询 `/api/admin/users` [GET]
- 参数（均可选，支持分页）：
  - username: 模糊匹配
  - email: 模糊匹配
  - role: 精确匹配
  - PageNum, PageSize
- 响应：分页信息 + 用户列表（userId, username, email, role, token_count, create_time）

### 3) 管理员模型查询 `/api/admin/models` [GET]
- 参数（均可选，支持分页）：
  - minLike, maxLike
  - minDownload, maxDownload
  - isPrivate: 0/1 或 true/false
  - PageNum, PageSize
- 响应：分页信息 + 模型列表（jobId, userId, like, downloadCount, Isprivate, status, resultFormat, version, create_time, prompt）

### 鉴权
- 以上接口均需请求头 `Session-Token`，且该 token 所属用户需具备 `role=admin`。

# API更新总结

## 主要更新内容

### 1. `/api/query` 接口增强

现在 `/api/query` 接口会返回数据库中的本地文件URL和私有状态信息：

#### 新增返回字段：
- `fileurl` - 本地模型文件URL（多个文件用逗号分隔）
- `previewImages` - 本地预览图片URL（多个图片用逗号分隔）
- `Isprivate` - 任务是否为私有状态（布尔值）
- `downloadCount` - 下载次数
- `like` - 收藏次数
- `createTime` - 创建时间

#### 响应示例：
```json
{
    "status": "success",
    "code": 200,
    "message": "分页查询任务成功",
    "data": {
        "pageInfo": {
            "pageNum": 1,
            "pageSize": 10,
            "totalCount": 5,
            "totalPage": 1
        },
        "list": [
            {
                "jobId": "example_job_123",
                "status": "DONE",
                "requestId": "req_123",
                "modelList": [
                    {
                        "fileUrl": "https://tencent-cloud-url/model.glb",
                        "fileFormat": "GLB"
                    }
                ],
                "previewImages": [
                    "https://tencent-cloud-url/preview.jpg"
                ],
                "fileurl": "/model/glb/1704067200000_1234.glb",
                "previewImages": "/model/glb/1704067200000_1234.jpg",
                "Isprivate": false,
                "downloadCount": 5,
                "like": 3,
                "createTime": "2024-01-01 12:00:00"
            }
        ]
    }
}
```

### 2. 新增下载接口

#### `/download/(.*)` - 文件下载接口

提供专门的文件下载服务，支持所有模型格式和预览图片。

**访问方式：**
```
GET /download/glb/1704067200000_1234.glb
GET /download/obj/1704067200001_5678.obj
GET /download/glb/1704067200000_1234.jpg
```

**特性：**
- 自动设置正确的MIME类型
- 设置 `Content-Disposition: attachment` 触发下载
- 支持所有支持的模型格式和图片格式
- 包含安全路径验证

### 3. 静态文件服务优化

#### `/model/(.*)` - 静态文件访问接口

优化了原有的静态文件服务，修复了 `res.set_content` 错误：

**访问方式：**
```
GET /model/glb/1704067200000_1234.glb
GET /model/obj/1704067200001_5678.obj
GET /model/glb/1704067200000_1234.jpg
```

**特性：**
- 修复了内存管理问题
- 使用缓冲区正确读取文件
- 设置 `Content-Disposition: inline` 用于在线预览
- 支持HTTP缓存（1小时）

### 4. 数据库查询优化

#### 新增函数：`getTaskCompleteInfo`

```cpp
Json::Value getTaskCompleteInfo(const std::string& jobId);
```

**功能：**
- 一次性获取任务的所有相关信息
- 包括文件URL、预览图片、私有状态、统计信息等
- 使用统一的数据库连接管理

## 文件访问方式对比

### 在线预览 vs 文件下载

| 接口 | 用途 | Content-Disposition | 适用场景 |
|------|------|-------------------|----------|
| `/model/...` | 在线预览 | `inline` | 网页中显示图片、模型预览 |
| `/download/...` | 文件下载 | `attachment` | 用户主动下载文件 |

### 示例对比

**在线预览（浏览器直接显示）：**
```
GET /model/glb/1704067200000_1234.jpg
Content-Disposition: inline; filename="1704067200000_1234.jpg"
```

**文件下载（触发下载对话框）：**
```
GET /download/glb/1704067200000_1234.glb
Content-Disposition: attachment; filename="1704067200000_1234.glb"
```

## 支持的MIME类型

| 文件格式 | MIME类型 |
|----------|----------|
| .obj | application/octet-stream |
| .fbx | application/octet-stream |
| .glb | model/gltf-binary |
| .stl | application/octet-stream |
| .usdz | model/vnd.usdz+zip |
| .mp4 | video/mp4 |
| .jpg/.jpeg | image/jpeg |
| .png | image/png |

## 安全特性

1. **路径验证**：确保只能访问 `/root/Qiniu/server/model/` 目录下的文件
2. **文件存在检查**：验证文件确实存在且为常规文件
3. **内存管理**：正确分配和释放文件读取缓冲区
4. **错误处理**：完善的错误响应和状态码

## 使用建议

### 前端集成

1. **显示预览图片**：使用 `/model/` 接口
   ```html
   <img src="/model/glb/1704067200000_1234.jpg" alt="模型预览">
   ```

2. **下载模型文件**：使用 `/download/` 接口
   ```javascript
   window.open('/download/glb/1704067200000_1234.glb');
   ```

3. **获取文件列表**：使用 `/api/query` 接口
   ```javascript
   fetch('/api/query?UserId=123&PageNum=1&PageSize=10')
     .then(response => response.json())
     .then(data => {
       // 处理返回的文件URL列表
       data.data.list.forEach(task => {
         console.log('模型文件:', task.fileurl);
         console.log('预览图片:', task.previewImages);
         console.log('是否私有:', task.Isprivate);
       });
     });
   ```

### 错误处理

所有接口都会返回适当的HTTP状态码：
- `200` - 成功
- `400` - 参数错误
- `403` - 访问被拒绝
- `404` - 文件不存在
- `500` - 服务器内部错误

## 向后兼容性

- 所有现有的API端点保持不变
- 只是增加了新的返回字段
- 客户端可以选择性地使用新增的字段
- 原有的腾讯云URL仍然可用作为备用
