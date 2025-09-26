#include "concurrent_downloader.h"
#include "config.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <thread>
#include <fstream>
#include <curl/curl.h>

// ConcurrentDownloader 实现
ConcurrentDownloader& ConcurrentDownloader::getInstance() {
    static ConcurrentDownloader instance;
    return instance;
}

void ConcurrentDownloader::initialize(int maxConcurrentDownloads) {
    maxConcurrentDownloads_ = maxConcurrentDownloads;
    shutdown_.store(false);
    
    // 启动工作线程
    for (int i = 0; i < maxConcurrentDownloads_; ++i) {
        workers_.emplace_back(&ConcurrentDownloader::downloadWorker, this);
    }
    
    std::cout << "并发下载器初始化完成，工作线程数: " << maxConcurrentDownloads_ << std::endl;
}

// ---- Local utilities (migrated) ----
namespace {
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        std::ofstream* file = static_cast<std::ofstream*>(userp);
        if (file && file->is_open()) {
            file->write(static_cast<char*>(contents), totalSize);
        }
        return totalSize;
    }

    static std::string getFolderPathByFormat(ModelFormat format) {
        std::string basePath = std::string(MODEL_FS_BASE_DIR) + "/";
        switch (format) {
            case ModelFormat::OBJ:  return basePath + "obj/";
            case ModelFormat::FBX:  return basePath + "fbx/";
            case ModelFormat::GLB:  return basePath + "glb/";
            case ModelFormat::STL:  return basePath + "stl/";
            case ModelFormat::USDZ: return basePath + "usdz/";
            case ModelFormat::MP4:  return basePath + "mp4/";
            default:                return basePath + "unknown/";
        }
    }

    static ModelFormat getModelFormatFromUrl(const std::string& url) {
        std::string lowerUrl = url;
        std::transform(lowerUrl.begin(), lowerUrl.end(), lowerUrl.begin(), ::tolower);
        if (lowerUrl.find(".obj")  != std::string::npos) return ModelFormat::OBJ;
        if (lowerUrl.find(".fbx")  != std::string::npos) return ModelFormat::FBX;
        if (lowerUrl.find(".glb")  != std::string::npos) return ModelFormat::GLB;
        if (lowerUrl.find(".stl")  != std::string::npos) return ModelFormat::STL;
        if (lowerUrl.find(".usdz") != std::string::npos) return ModelFormat::USDZ;
        if (lowerUrl.find(".mp4")  != std::string::npos) return ModelFormat::MP4;
        return ModelFormat::UNKNOWN;
    }

    static std::string generateUniqueFileName(const std::string& extension) {
        auto now = std::chrono::high_resolution_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        std::random_device rd; std::mt19937 gen(rd()); std::uniform_int_distribution<> dis(1000, 9999);
        int randomNum = dis(gen);
        std::ostringstream oss; oss << timestamp << "_" << randomNum;
        if (!extension.empty() && extension[0] != '.') oss << "." << extension; else if (!extension.empty()) oss << extension;
        return oss.str();
    }

    static std::string generateModelFilePath(ModelFormat format) {
        std::string folderAbs = getFolderPathByFormat(format);
        if (!std::filesystem::exists(folderAbs)) {
            std::filesystem::create_directories(folderAbs);
        }
        std::string extension;
        switch (format) {
            case ModelFormat::OBJ: extension = ".obj"; break;
            case ModelFormat::FBX: extension = ".fbx"; break;
            case ModelFormat::GLB: extension = ".glb"; break;
            case ModelFormat::STL: extension = ".stl"; break;
            case ModelFormat::USDZ: extension = ".usdz"; break;
            case ModelFormat::MP4: extension = ".mp4"; break;
            default: extension = ".bin"; break;
        }
        std::string fileName = generateUniqueFileName(extension);
        std::string absCopy = folderAbs; if (!absCopy.empty() && absCopy.back() == '/') absCopy.pop_back();
        size_t slashPos = absCopy.find_last_of('/');
        std::string fmtDir = (slashPos == std::string::npos) ? std::string("") : absCopy.substr(slashPos + 1);
        return std::string(MODEL_URL_BASE_PATH) + "/" + fmtDir + "/" + fileName;
    }

    static bool downloadFileFromUrl(const std::string& url, const std::string& localPath) {
        std::filesystem::path p(localPath);
        std::filesystem::create_directories(p.parent_path());
        CURL* curl = curl_easy_init();
        if (!curl) { std::cerr << "CURL初始化失败" << std::endl; return false; }
        std::ofstream file(localPath, std::ios::binary);
        if (!file.is_open()) { std::cerr << "无法创建文件: " << localPath << std::endl; curl_easy_cleanup(curl); return false; }
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, MODEL_DOWNLOAD_TIMEOUT_SECONDS);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "ModelDownloader/1.0");
        CURLcode res = curl_easy_perform(curl);
        file.close(); curl_easy_cleanup(curl);
        if (res != CURLE_OK) { std::cerr << "下载失败: " << curl_easy_strerror(res) << std::endl; std::remove(localPath.c_str()); return false; }
        if (std::filesystem::exists(localPath)) {
            auto fileSize = std::filesystem::file_size(localPath);
            if (fileSize == 0) { std::cerr << "下载的文件为空" << std::endl; std::remove(localPath.c_str()); return false; }
        }
        return true;
    }
}
// ---- end Local utilities ----
std::string ConcurrentDownloader::addDownloadTask(const std::string& url, const std::string& localPath) {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    std::string taskId = generateTaskId();
    auto task = std::make_shared<DownloadTask>();
    task->url = url;
    task->localPath = localPath;
    task->taskId = taskId;
    task->status = DownloadStatus::PENDING;
    task->startTime = std::chrono::steady_clock::now();
    
    tasks_[taskId] = task;
    pendingTasks_.push(taskId);
    totalTasks_++;
    
    taskCondition_.notify_one();
    
    return taskId;
}

std::vector<std::string> ConcurrentDownloader::addBatchDownloadTasks(
    const std::vector<std::pair<std::string, std::string>>& tasks) {
    
    std::vector<std::string> taskIds;
    taskIds.reserve(tasks.size());
    
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    for (const auto& task : tasks) {
        std::string taskId = generateTaskId();
        auto downloadTask = std::make_shared<DownloadTask>();
        downloadTask->url = task.first;
        downloadTask->localPath = task.second;
        downloadTask->taskId = taskId;
        downloadTask->status = DownloadStatus::PENDING;
        downloadTask->startTime = std::chrono::steady_clock::now();
        
        tasks_[taskId] = downloadTask;
        pendingTasks_.push(taskId);
        totalTasks_++;
        taskIds.push_back(taskId);
    }
    
    taskCondition_.notify_all();
    
    return taskIds;
}

DownloadStatus ConcurrentDownloader::getTaskStatus(const std::string& taskId) {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    auto it = tasks_.find(taskId);
    if (it != tasks_.end()) {
        return it->second->status;
    }
    
    return DownloadStatus::FAILED;
}

ConcurrentDownloader::TaskProgress ConcurrentDownloader::getTaskProgress(const std::string& taskId) {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    TaskProgress progress;
    auto it = tasks_.find(taskId);
    
    if (it != tasks_.end()) {
        const auto& task = it->second;
        progress.downloadedBytes = task->downloadedBytes.load();
        progress.totalBytes = task->totalBytes.load();
        progress.errorMessage = task->errorMessage;
        
        if (progress.totalBytes > 0) {
            progress.progress = static_cast<double>(progress.downloadedBytes) / progress.totalBytes;
        } else {
            progress.progress = 0.0;
        }
        
        switch (task->status) {
            case DownloadStatus::PENDING:
                progress.status = "pending";
                break;
            case DownloadStatus::DOWNLOADING:
                progress.status = "downloading";
                break;
            case DownloadStatus::COMPLETED:
                progress.status = "completed";
                progress.progress = 1.0;
                break;
            case DownloadStatus::FAILED:
                progress.status = "failed";
                break;
        }
    } else {
        progress.status = "not_found";
        progress.progress = 0.0;
    }
    
    return progress;
}

bool ConcurrentDownloader::waitForTask(const std::string& taskId, int timeoutSeconds) {
    auto startTime = std::chrono::steady_clock::now();
    
    while (true) {
        {
            std::lock_guard<std::mutex> lock(tasksMutex_);
            auto it = tasks_.find(taskId);
            if (it != tasks_.end()) {
                auto status = it->second->status;
                if (status == DownloadStatus::COMPLETED || status == DownloadStatus::FAILED) {
                    return status == DownloadStatus::COMPLETED;
                }
            } else {
                return false; // 任务不存在
            }
        }
        
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        
        if (elapsed >= timeoutSeconds) {
            return false; // 超时
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ConcurrentDownloader::waitForAllTasks() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(tasksMutex_);
            bool allCompleted = true;
            
            for (const auto& pair : tasks_) {
                auto status = pair.second->status;
                if (status != DownloadStatus::COMPLETED && status != DownloadStatus::FAILED) {
                    allCompleted = false;
                    break;
                }
            }
            
            if (allCompleted) {
                break;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

bool ConcurrentDownloader::cancelTask(const std::string& taskId) {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    auto it = tasks_.find(taskId);
    if (it != tasks_.end() && it->second->status == DownloadStatus::PENDING) {
        it->second->status = DownloadStatus::FAILED;
        it->second->errorMessage = "Cancelled by user";
        return true;
    }
    
    return false;
}

ConcurrentDownloader::DownloadStats ConcurrentDownloader::getStats() {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    DownloadStats stats;
    stats.totalTasks = totalTasks_.load();
    stats.completedTasks = completedTasks_.load();
    stats.failedTasks = failedTasks_.load();
    stats.totalDownloadedBytes = totalDownloadedBytes_.load();
    
    int pending = 0;
    int downloading = 0;
    
    for (const auto& pair : tasks_) {
        switch (pair.second->status) {
            case DownloadStatus::PENDING:
                pending++;
                break;
            case DownloadStatus::DOWNLOADING:
                downloading++;
                break;
            default:
                break;
        }
    }
    
    stats.pendingTasks = pending;
    stats.downloadingTasks = downloading;
    
    // 计算平均速度（简化计算）
    stats.averageSpeed = 0.0; // 可以基于历史数据计算
    
    return stats;
}

void ConcurrentDownloader::cleanupCompletedTasks() {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    auto it = tasks_.begin();
    while (it != tasks_.end()) {
        auto status = it->second->status;
        if (status == DownloadStatus::COMPLETED || status == DownloadStatus::FAILED) {
            it = tasks_.erase(it);
        } else {
            ++it;
        }
    }
}

void ConcurrentDownloader::shutdown() {
    shutdown_.store(true);
    taskCondition_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
}

ConcurrentDownloader::~ConcurrentDownloader() {
    shutdown();
}

void ConcurrentDownloader::downloadWorker() {
    while (!shutdown_.load()) {
        std::string taskId;
        
        {
            std::unique_lock<std::mutex> lock(tasksMutex_);
            taskCondition_.wait(lock, [this] { 
                return shutdown_.load() || !pendingTasks_.empty(); 
            });
            
            if (shutdown_.load()) {
                break;
            }
            
            if (!pendingTasks_.empty()) {
                taskId = pendingTasks_.front();
                pendingTasks_.pop();
            }
        }
        
        if (!taskId.empty()) {
            std::shared_ptr<DownloadTask> task;
            {
                std::lock_guard<std::mutex> lock(tasksMutex_);
                auto it = tasks_.find(taskId);
                if (it != tasks_.end()) {
                    task = it->second;
                    task->status = DownloadStatus::DOWNLOADING;
                }
            }
            
            if (task) {
                bool success = downloadFile(*task);
                
                {
                    std::lock_guard<std::mutex> lock(tasksMutex_);
                    if (success) {
                        task->status = DownloadStatus::COMPLETED;
                        completedTasks_++;
                        totalDownloadedBytes_ += task->downloadedBytes.load();
                    } else {
                        task->status = DownloadStatus::FAILED;
                        failedTasks_++;
                    }
                    task->endTime = std::chrono::steady_clock::now();
                }
            }
        }
    }
}

std::string ConcurrentDownloader::generateTaskId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    for (int i = 0; i < 16; ++i) {
        ss << std::hex << dis(gen);
    }
    
    return ss.str();
}

bool ConcurrentDownloader::downloadFile(const DownloadTask& task) {
    try {
        return downloadFileFromUrl(task.url, task.localPath);
    } catch (const std::exception& e) {
        std::cerr << "下载文件异常: " << e.what() << std::endl;
        return false;
    }
}

// OptimizedModelDownloader 实现
OptimizedModelDownloader& OptimizedModelDownloader::getInstance() {
    static OptimizedModelDownloader instance;
    return instance;
}

OptimizedModelDownloader::OptimizedModelDownloader() : downloader_(ConcurrentDownloader::getInstance()) {
}

Json::Value OptimizedModelDownloader::downloadModelFilesConcurrent(const Json::Value& modelList, const Json::Value& previewImages) {
    Json::Value result;
    Json::Value downloadedModels(Json::arrayValue);
    Json::Value downloadedPreviews(Json::arrayValue);
    
    try {
        // 准备下载任务
    std::vector<std::pair<std::string, std::string>> downloadTasks; // url, absolute local path
    std::vector<std::string> relativePaths; // relative local paths for API response
        
        // 添加模型文件下载任务
        for (const auto& model : modelList) {
            if (model.isObject() && model.isMember("fileUrl") && model.isMember("fileFormat")) {
                std::string url = model["fileUrl"].asString();
                std::string format = model["fileFormat"].asString();
                
                ModelFormat modelFormat = ModelFormat::UNKNOWN;
                std::string lowerFormat = format;
                std::transform(lowerFormat.begin(), lowerFormat.end(), lowerFormat.begin(), ::tolower);
                
                if (lowerFormat == "obj") modelFormat = ModelFormat::OBJ;
                else if (lowerFormat == "fbx") modelFormat = ModelFormat::FBX;
                else if (lowerFormat == "glb") modelFormat = ModelFormat::GLB;
                else if (lowerFormat == "stl") modelFormat = ModelFormat::STL;
                else if (lowerFormat == "usdz") modelFormat = ModelFormat::USDZ;
                else if (lowerFormat == "mp4") modelFormat = ModelFormat::MP4;
                
    // 生成保存路径（URL 形式），并确保目录存在
    std::string relativePath = generateModelFilePath(modelFormat);
    // 将 URL 路径映射为物理存储路径
    std::string rel = relativePath;
    if (rel.rfind(MODEL_URL_BASE_PATH, 0) == 0) {
        rel = rel.substr(std::string(MODEL_URL_BASE_PATH).size());
    }
    if (!rel.empty() && rel[0] != '/') rel = std::string("/") + rel;
    std::string absolutePath = std::string(MODEL_FS_BASE_DIR) + rel;
                downloadTasks.emplace_back(url, absolutePath);
                relativePaths.emplace_back(relativePath);
            }
        }
        
        // 批量提交下载任务
        auto taskIds = downloader_.addBatchDownloadTasks(downloadTasks);
        
        // 等待所有模型文件下载完成
        for (size_t i = 0; i < taskIds.size(); ++i) {
            const auto& taskId = taskIds[i];
            if (downloader_.waitForTask(taskId, 300)) { // 5分钟超时
                // 下载成功，添加到结果中
                Json::Value downloadedModel;
                downloadedModel["fileUrl"] = relativePaths[i];
                downloadedModel["fileFormat"] = modelList[static_cast<Json::ArrayIndex>(i)]["fileFormat"];
                if (modelList[static_cast<Json::ArrayIndex>(i)].isMember("requestId")) {
                    downloadedModel["requestId"] = modelList[static_cast<Json::ArrayIndex>(i)]["requestId"];
                }
                downloadedModels.append(downloadedModel);
            }
        }
        
        // 下载预览图片
        int previewIndex = 0;
        for (const auto& preview : previewImages) {
            if (preview.isString() && previewIndex < downloadedModels.size()) {
                std::string url = preview.asString();
                std::string modelFilePath = downloadedModels[previewIndex]["fileUrl"].asString();
                
                std::string localPath = downloadPreviewImageConcurrent(url, modelFilePath);
                if (!localPath.empty()) {
                    downloadedPreviews.append(localPath);
                }
            }
            previewIndex++;
        }
        
        result["modelList"] = downloadedModels;
        result["previewImages"] = downloadedPreviews;
        result["success"] = true;
        
    } catch (const std::exception& e) {
        std::cerr << "并发下载模型文件异常: " << e.what() << std::endl;
        result["success"] = false;
        result["error"] = e.what();
    }
    
    return result;
}

std::string OptimizedModelDownloader::downloadModelFileConcurrent(const std::string& url, ModelFormat format) {
    std::string relativePath = generateModelFilePath(format);
    std::string rel = relativePath;
    if (rel.rfind(MODEL_URL_BASE_PATH, 0) == 0) rel = rel.substr(std::string(MODEL_URL_BASE_PATH).size());
    if (!rel.empty() && rel[0] != '/') rel = std::string("/") + rel;
    std::string localPath = std::string(MODEL_FS_BASE_DIR) + rel;
    
    std::string taskId = downloader_.addDownloadTask(url, localPath);
    if (downloader_.waitForTask(taskId, MODEL_DOWNLOAD_TIMEOUT_SECONDS)) {
        return relativePath;
    }
    
    return "";
}

std::string OptimizedModelDownloader::downloadPreviewImageConcurrent(const std::string& url, const std::string& baseFileName) {
    // 从基础文件名提取目录和文件名
    std::string::size_type lastSlash = baseFileName.find_last_of("/");
    // 将 URL 路径映射为物理存储路径
    std::string urlBase = std::string(MODEL_URL_BASE_PATH);
    std::string relDir = baseFileName.substr(0, lastSlash + 1);
    if (relDir.rfind(urlBase, 0) == 0) {
        relDir = relDir.substr(urlBase.size());
    }
    if (!relDir.empty() && relDir[0] != '/') relDir = std::string("/") + relDir;
    std::string directory = std::string(MODEL_FS_BASE_DIR) + relDir;
    std::string fileName = baseFileName.substr(lastSlash + 1);
    
    // 移除扩展名，保留基础名称
    std::string::size_type lastDot = fileName.find_last_of(".");
    if (lastDot != std::string::npos) {
        fileName = fileName.substr(0, lastDot);
    }
    
    // 添加.jpg扩展名
    fileName += ".jpg";
    std::string localPath = directory + fileName;
    
    // 确保目录存在
    if (!std::filesystem::exists(directory)) {
        std::filesystem::create_directories(directory);
    }
    
    // 并发下载
    std::string taskId = downloader_.addDownloadTask(url, localPath);
    if (downloader_.waitForTask(taskId, MODEL_DOWNLOAD_TIMEOUT_SECONDS/2)) { // 预览图给更短超时
        // 返回相对于服务器根的URL
        return baseFileName.substr(0, lastSlash + 1) + fileName;
    }
    
    return "";
}

Json::Value OptimizedModelDownloader::getDownloadProgress(const std::vector<std::string>& taskIds) {
    Json::Value result(Json::arrayValue);
    
    for (const auto& taskId : taskIds) {
        auto progress = downloader_.getTaskProgress(taskId);
        
        Json::Value taskProgress;
        taskProgress["taskId"] = taskId;
        taskProgress["downloadedBytes"] = static_cast<Json::Int64>(progress.downloadedBytes);
        taskProgress["totalBytes"] = static_cast<Json::Int64>(progress.totalBytes);
        taskProgress["progress"] = progress.progress;
        taskProgress["status"] = progress.status;
        taskProgress["errorMessage"] = progress.errorMessage;
        
        result.append(taskProgress);
    }
    
    return result;
}

// 全局函数实现
bool initializeConcurrentDownloader(int maxConcurrentDownloads) {
    ConcurrentDownloader::getInstance().initialize(maxConcurrentDownloads);
    return true;
}

ConcurrentDownloader& getConcurrentDownloader() {
    return ConcurrentDownloader::getInstance();
}

OptimizedModelDownloader& getOptimizedModelDownloader() {
    return OptimizedModelDownloader::getInstance();
}
