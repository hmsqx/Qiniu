#include "model_downloader.h"
#include "config.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <chrono>
#include <filesystem>
#include <curl/curl.h>
#include <algorithm>

namespace fs = std::filesystem;

// CURL写入回调函数
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::ofstream* file = static_cast<std::ofstream*>(userp);
    if (file && file->is_open()) {
        file->write(static_cast<char*>(contents), totalSize);
    }
    return totalSize;
}

ModelFormat getModelFormatFromUrl(const std::string& url) {
    // 转换为小写进行比较
    std::string lowerUrl = url;
    std::transform(lowerUrl.begin(), lowerUrl.end(), lowerUrl.begin(), ::tolower);
    
    if (lowerUrl.find(".obj") != std::string::npos) {
        return ModelFormat::OBJ;
    } else if (lowerUrl.find(".fbx") != std::string::npos) {
        return ModelFormat::FBX;
    } else if (lowerUrl.find(".glb") != std::string::npos) {
        return ModelFormat::GLB;
    } else if (lowerUrl.find(".stl") != std::string::npos) {
        return ModelFormat::STL;
    } else if (lowerUrl.find(".usdz") != std::string::npos) {
        return ModelFormat::USDZ;
    } else if (lowerUrl.find(".mp4") != std::string::npos) {
        return ModelFormat::MP4;
    }
    
    return ModelFormat::UNKNOWN;
}

std::string getFolderPathByFormat(ModelFormat format) {
    std::string basePath = "/root/Qiniu/server/model/";
    
    switch (format) {
        case ModelFormat::OBJ:
            return basePath + "obj/";
        case ModelFormat::FBX:
            return basePath + "fbx/";
        case ModelFormat::GLB:
            return basePath + "glb/";
        case ModelFormat::STL:
            return basePath + "stl/";
        case ModelFormat::USDZ:
            return basePath + "usdz/";
        case ModelFormat::MP4:
            return basePath + "mp4/";
        default:
            return basePath + "unknown/";
    }
}

std::string generateUniqueFileName(const std::string& extension) {
    // 获取当前时间戳
    auto now = std::chrono::high_resolution_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    // 生成随机数
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    int randomNum = dis(gen);
    
    // 组合文件名：时间戳_随机数.扩展名
    std::ostringstream oss;
    oss << timestamp << "_" << randomNum;
    if (!extension.empty() && extension[0] != '.') {
        oss << "." << extension;
    } else if (!extension.empty()) {
        oss << extension;
    }
    
    return oss.str();
}

bool downloadFileFromUrl(const std::string& url, const std::string& localPath) {
    CURL* curl;
    CURLcode res;
    
    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL初始化失败" << std::endl;
        return false;
    }
    
    std::ofstream file(localPath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法创建文件: " << localPath << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }
    
    // 设置CURL选项
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L); // 5分钟超时
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ModelDownloader/1.0");
    
    // 执行下载
    res = curl_easy_perform(curl);
    
    file.close();
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        std::cerr << "下载失败: " << curl_easy_strerror(res) << std::endl;
        // 删除失败的文件
        std::remove(localPath.c_str());
        return false;
    }
    
    // 检查文件大小
    if (fs::exists(localPath)) {
        auto fileSize = fs::file_size(localPath);
        if (fileSize == 0) {
            std::cerr << "下载的文件为空" << std::endl;
            std::remove(localPath.c_str());
            return false;
        }
        std::cout << "成功下载文件: " << localPath << " (大小: " << fileSize << " 字节)" << std::endl;
        return true;
    }
    
    return false;
}

std::string downloadModelFile(const std::string& url, ModelFormat format) {
    // 确保目录存在
    std::string folderPath = getFolderPathByFormat(format);
    if (!fs::exists(folderPath)) {
        fs::create_directories(folderPath);
    }
    
    // 生成文件名
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
    std::string localPath = folderPath + fileName;
    
    // 下载文件
    if (downloadFileFromUrl(url, localPath)) {
        // 返回相对于服务器根路径的URL
        return "/model/" + getFolderPathByFormat(format).substr(getFolderPathByFormat(format).find("/model/") + 7) + fileName;
    }
    
    return "";
}

std::string downloadPreviewImage(const std::string& url, const std::string& baseFileName) {
    // 从基础文件名中提取目录路径和文件名（不含扩展名）
    std::string::size_type lastSlash = baseFileName.find_last_of("/");
    std::string directory = "/root/Qiniu/server" + baseFileName.substr(0, lastSlash + 1);
    std::string fileName = baseFileName.substr(lastSlash + 1);
    
    // 移除扩展名，只保留基础名称
    std::string::size_type lastDot = fileName.find_last_of(".");
    if (lastDot != std::string::npos) {
        fileName = fileName.substr(0, lastDot);
    }
    
    // 添加.jpg扩展名
    fileName += ".jpg";
    std::string localPath = directory + fileName;
    
    // 下载文件
    if (downloadFileFromUrl(url, localPath)) {
        // 返回相对于服务器根路径的URL
        return baseFileName.substr(0, lastSlash + 1) + fileName;
    }
    
    return "";
}

Json::Value downloadModelFiles(const Json::Value& modelList, const Json::Value& previewImages) {
    Json::Value result;
    Json::Value downloadedModels(Json::arrayValue);
    Json::Value downloadedPreviews(Json::arrayValue);
    
    // 下载模型文件
    for (const auto& model : modelList) {
        if (model.isObject() && model.isMember("fileUrl") && model.isMember("fileFormat")) {
            std::string url = model["fileUrl"].asString();
            std::string format = model["fileFormat"].asString();
            
            // 转换格式字符串为枚举
            ModelFormat modelFormat = ModelFormat::UNKNOWN;
            std::string lowerFormat = format;
            std::transform(lowerFormat.begin(), lowerFormat.end(), lowerFormat.begin(), ::tolower);
            
            if (lowerFormat == "obj") modelFormat = ModelFormat::OBJ;
            else if (lowerFormat == "fbx") modelFormat = ModelFormat::FBX;
            else if (lowerFormat == "glb") modelFormat = ModelFormat::GLB;
            else if (lowerFormat == "stl") modelFormat = ModelFormat::STL;
            else if (lowerFormat == "usdz") modelFormat = ModelFormat::USDZ;
            else if (lowerFormat == "mp4") modelFormat = ModelFormat::MP4;
            
            std::string localPath = downloadModelFile(url, modelFormat);
            if (!localPath.empty()) {
                Json::Value downloadedModel;
                downloadedModel["fileUrl"] = localPath;  // 本地路径
                downloadedModel["fileFormat"] = format;
                if (model.isMember("requestId")) {
                    downloadedModel["requestId"] = model["requestId"];
                }
                downloadedModels.append(downloadedModel);
            } else {
                std::cerr << "下载模型文件失败: " << url << std::endl;
            }
        }
    }
    
    // 下载预览图片，与对应的模型文件放在同一目录
    int previewIndex = 0;
    for (const auto& preview : previewImages) {
        if (preview.isString() && previewIndex < downloadedModels.size()) {
            std::string url = preview.asString();
            std::string modelFilePath = downloadedModels[previewIndex]["fileUrl"].asString();
            
            std::string localPath = downloadPreviewImage(url, modelFilePath);
            if (!localPath.empty()) {
                downloadedPreviews.append(localPath);
            } else {
                std::cerr << "下载预览图片失败: " << url << std::endl;
            }
        }
        previewIndex++;
    }
    
    result["modelList"] = downloadedModels;
    result["previewImages"] = downloadedPreviews;
    result["success"] = true;
    
    return result;
}
