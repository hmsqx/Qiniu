# 静态文件服务器配置说明

## 模型文件目录结构

```
/root/Qiniu/server/model/
├── obj/          # OBJ格式模型文件
├── fbx/          # FBX格式模型文件  
├── glb/          # GLB格式模型文件
├── stl/          # STL格式模型文件
├── usdz/         # USDZ格式模型文件
├── mp4/          # MP4视频文件
└── preview/      # 预览图片文件
```

## HTTP服务器配置

为了能够通过HTTP访问下载的模型文件，需要在主程序中添加静态文件服务功能。

### 建议的实现方式

1. 使用httplib的静态文件服务功能
2. 在main.cc中添加静态文件路由
3. 设置正确的MIME类型

### 示例代码

```cpp
// 在main.cc中添加
server.set_mount_point("/model", "/root/Qiniu/server/model");

// 或者手动处理静态文件请求
server.Get("/model/.*", [](const httplib::Request &req, httplib::Response &res) {
    // 处理模型文件请求
    std::string filePath = "/root/Qiniu/server" + req.path;
    
    // 检查文件是否存在
    if (std::filesystem::exists(filePath)) {
        // 根据文件扩展名设置正确的MIME类型
        std::string ext = std::filesystem::path(filePath).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        if (ext == ".obj") res.set_header("Content-Type", "application/octet-stream");
        else if (ext == ".fbx") res.set_header("Content-Type", "application/octet-stream");
        else if (ext == ".glb") res.set_header("Content-Type", "model/gltf-binary");
        else if (ext == ".stl") res.set_header("Content-Type", "application/octet-stream");
        else if (ext == ".usdz") res.set_header("Content-Type", "model/vnd.usdz+zip");
        else if (ext == ".mp4") res.set_header("Content-Type", "video/mp4");
        else if (ext == ".jpg" || ext == ".jpeg") res.set_header("Content-Type", "image/jpeg");
        else if (ext == ".png") res.set_header("Content-Type", "image/png");
        
        // 读取并返回文件内容
        std::ifstream file(filePath, std::ios::binary);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            res.set_content(content, res.get_header_value("Content-Type"));
        } else {
            res.status = 404;
            res.set_content("文件未找到", "text/plain");
        }
    } else {
        res.status = 404;
        res.set_content("文件未找到", "text/plain");
    }
});
```

## 文件访问URL格式

下载的模型文件可以通过以下URL格式访问：

- 模型文件: `http://服务器地址:8080/model/格式/文件名`
- 预览图片: `http://服务器地址:8080/model/preview/文件名`

例如：
- `http://localhost:8080/model/obj/1234567890_1234.obj`
- `http://localhost:8080/model/preview/1234567890_5678.jpg`

## 安全考虑

1. 限制访问的文件类型
2. 验证文件路径，防止目录遍历攻击
3. 设置适当的缓存头
4. 考虑添加访问控制（如果需要）
