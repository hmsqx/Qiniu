# 重复下载问题修复

## 问题描述

在之前的实现中，`/api/query` 接口存在重复下载问题：

1. `getTaskCompleteInfo` 从数据库获取已存在的本地文件URL
2. `queryTaskStatusFromTx` 查询腾讯云任务状态时，如果任务状态为 "DONE"，会重新下载文件
3. 这导致服务器上存在重复的文件，只是文件名不同
4. 浪费存储空间和网络带宽

## 问题原因

在 `queryTaskStatusFromTx`、`queryTaskStatusFromTxPro`、`queryTaskStatusFromTxRapid` 函数中，每次任务状态为 "DONE" 时都会无条件地执行下载逻辑，没有检查数据库中是否已经存在本地文件URL。

## 解决方案

### 1. 添加重复检查逻辑

在三个查询函数中添加了以下逻辑：

```cpp
// 检查数据库中是否已经存在本地文件URL
Json::Value existingFiles = getTaskFileInfo(jobId);
if (existingFiles.get("found", false).asBool()) {
    std::string existingFileUrls = existingFiles.get("fileurl", "").asString();
    std::string existingPreviewUrls = existingFiles.get("previewImages", "").asString();
    
    // 如果数据库中已经有本地文件URL，则使用现有的，不重新下载
    if (!existingFileUrls.empty()) {
        std::cout << "任务 " << jobId << " 已存在本地文件，跳过下载" << std::endl;
        // 使用现有的本地URL...
    } else {
        // 数据库中不存在本地文件URL，进行下载
        downloadAndSaveFiles(jobId, modelList, previewList, taskInfo);
    }
} else {
    // 数据库中不存在任务记录，进行下载
    downloadAndSaveFiles(jobId, modelList, previewList, taskInfo);
}
```

### 2. 创建辅助函数

添加了 `downloadAndSaveFiles` 辅助函数，将下载和保存逻辑封装起来，避免代码重复：

```cpp
static void downloadAndSaveFiles(const std::string& jobId, 
                                const Json::Value& modelList, 
                                const Json::Value& previewList,
                                Json::Value& taskInfo)
{
    // 下载文件并保存到数据库的逻辑
}
```

### 3. 智能文件URL解析

当使用现有文件时，系统会：

1. 从数据库读取已保存的本地文件URL字符串
2. 按逗号分割解析出多个文件URL
3. 从URL中提取文件格式信息
4. 重新构建JSON结构返回给客户端

## 修改的文件

### `src/tx_ai3d.cc`

- 修改了 `queryTaskStatusFromTx` 函数
- 修改了 `queryTaskStatusFromTxPro` 函数  
- 修改了 `queryTaskStatusFromTxRapid` 函数
- 添加了 `downloadAndSaveFiles` 辅助函数
- 添加了必要的头文件引用

## 工作流程

### 修复前的工作流程

```
1. getTaskCompleteInfo() -> 获取数据库中的本地文件URL
2. queryTaskStatusFromTx() -> 查询腾讯云状态
3. 如果状态为 "DONE" -> 无条件下载文件
4. 结果：重复文件，浪费资源
```

### 修复后的工作流程

```
1. getTaskCompleteInfo() -> 获取数据库中的本地文件URL
2. queryTaskStatusFromTx() -> 查询腾讯云状态
3. 如果状态为 "DONE" -> 检查数据库是否已有本地文件URL
4. 如果已有 -> 使用现有文件，跳过下载
5. 如果没有 -> 下载文件并保存到数据库
6. 结果：避免重复下载，节省资源
```

## 日志输出

修复后的系统会输出清晰的日志信息：

- `"任务 XXX 已存在本地文件，跳过下载"` - 当使用现有文件时
- `"任务 XXX 不存在本地文件，开始下载"` - 当需要下载时
- `"任务 XXX 不存在数据库记录，开始下载"` - 当数据库中没有记录时

## 优势

1. **避免重复下载**：不再为同一个任务重复下载文件
2. **节省存储空间**：避免服务器上存在重复文件
3. **提高性能**：减少不必要的网络请求和文件操作
4. **节省带宽**：避免重复的网络传输
5. **保持一致性**：确保同一任务始终使用相同的本地文件URL

## 向后兼容性

- 所有现有的API接口保持不变
- 客户端代码无需修改
- 只是内部逻辑优化，对外接口一致

## 测试验证

创建了测试脚本 `test_duplicate_download_fix.sh` 来验证修复效果：

```bash
./test_duplicate_download_fix.sh
```

测试场景：
1. 第一次查询任务 - 应该下载文件到本地
2. 第二次查询相同任务 - 应该跳过下载，使用现有文件

## 注意事项

1. **数据库一致性**：确保数据库中的文件URL记录准确
2. **文件完整性**：如果本地文件被意外删除，需要重新下载
3. **错误处理**：下载失败时仍会返回腾讯云的原始URL作为备用

## 相关文件

- `src/tx_ai3d.cc` - 主要修改文件
- `src/db_utils.cc` - 数据库查询函数
- `test_duplicate_download_fix.sh` - 测试脚本
- `DUPLICATE_DOWNLOAD_FIX.md` - 本文档

现在系统已经修复了重复下载问题，能够智能地检查并重用已存在的本地文件，大大提高了效率和资源利用率。
