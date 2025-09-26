#pragma once

#include <string>
#include <vector>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <queue>
#include <thread>
#include "jsoncpp/json/json.h"
// 支持的模型格式
enum class ModelFormat {
    OBJ,
    FBX,
    GLB,
    STL,
    USDZ,
    MP4,
    UNKNOWN
};

// 下载任务状态
enum class DownloadStatus {
    PENDING,
    DOWNLOADING,
    COMPLETED,
    FAILED
};

// 下载任务信息
struct DownloadTask {
    std::string url;
    std::string localPath;
    std::string taskId;
    DownloadStatus status;
    std::string errorMessage;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
    std::atomic<size_t> downloadedBytes{0};
    std::atomic<size_t> totalBytes{0};
};

// 并发下载管理器
class ConcurrentDownloader {
public:
    static ConcurrentDownloader& getInstance();
    
    // 初始化下载器
    void initialize(int maxConcurrentDownloads = 5);
    
    // 添加下载任务
    std::string addDownloadTask(const std::string& url, const std::string& localPath);
    
    // 批量添加下载任务
    std::vector<std::string> addBatchDownloadTasks(const std::vector<std::pair<std::string, std::string>>& tasks);
    
    // 获取任务状态
    DownloadStatus getTaskStatus(const std::string& taskId);
    
    // 获取任务进度
    struct TaskProgress {
        size_t downloadedBytes;
        size_t totalBytes;
        double progress; // 0.0 - 1.0
        std::string status;
        std::string errorMessage;
    };
    TaskProgress getTaskProgress(const std::string& taskId);
    
    // 等待任务完成
    bool waitForTask(const std::string& taskId, int timeoutSeconds = 300);
    
    // 等待所有任务完成
    void waitForAllTasks();
    
    // 取消任务
    bool cancelTask(const std::string& taskId);
    
    // 获取统计信息
    struct DownloadStats {
        int totalTasks;
        int completedTasks;
        int failedTasks;
        int pendingTasks;
        int downloadingTasks;
        size_t totalDownloadedBytes;
        double averageSpeed; // bytes per second
    };
    DownloadStats getStats();
    
    // 清理完成的任务
    void cleanupCompletedTasks();
    
    // 停止下载器
    void shutdown();

private:
    ConcurrentDownloader() = default;
    ~ConcurrentDownloader();
    
    // 禁用拷贝
    ConcurrentDownloader(const ConcurrentDownloader&) = delete;
    ConcurrentDownloader& operator=(const ConcurrentDownloader&) = delete;
    
    // 工作线程函数
    void downloadWorker();
    
    // 生成任务ID
    std::string generateTaskId();
    
    // 下载单个文件
    bool downloadFile(const DownloadTask& task);
    
    // 配置
    int maxConcurrentDownloads_;
    std::atomic<bool> shutdown_{false};
    
    // 任务管理
    mutable std::mutex tasksMutex_;
    std::unordered_map<std::string, std::shared_ptr<DownloadTask>> tasks_;
    std::queue<std::string> pendingTasks_;
    std::condition_variable taskCondition_;
    
    // 工作线程
    std::vector<std::thread> workers_;
    
    // 统计信息
    std::atomic<int> totalTasks_{0};
    std::atomic<int> completedTasks_{0};
    std::atomic<int> failedTasks_{0};
    std::atomic<size_t> totalDownloadedBytes_{0};
};

// 优化的模型文件下载器
class OptimizedModelDownloader {
public:
    static OptimizedModelDownloader& getInstance();
    
    // 下载模型文件（并发版本）
    Json::Value downloadModelFilesConcurrent(const Json::Value& modelList, const Json::Value& previewImages);
    
    // 下载单个模型文件
    std::string downloadModelFileConcurrent(const std::string& url, ModelFormat format);
    
    // 下载预览图片（并发版本）
    std::string downloadPreviewImageConcurrent(const std::string& url, const std::string& baseFileName);
    
    // 获取下载进度
    Json::Value getDownloadProgress(const std::vector<std::string>& taskIds);

private:
    OptimizedModelDownloader();
    
    ConcurrentDownloader& downloader_;
};

// 全局函数
bool initializeConcurrentDownloader(int maxConcurrentDownloads = 5);
ConcurrentDownloader& getConcurrentDownloader();
OptimizedModelDownloader& getOptimizedModelDownloader();
