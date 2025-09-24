#pragma once

#include <string>
#include <jsoncpp/json/json.h>
#include <vector>

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

// 从URL获取文件扩展名并确定格式
ModelFormat getModelFormatFromUrl(const std::string& url);

// 根据格式获取对应的文件夹路径
std::string getFolderPathByFormat(ModelFormat format);

// 生成随机文件名，确保不重复
std::string generateUniqueFileName(const std::string& extension);

// 从腾讯云下载文件到本地
bool downloadFileFromUrl(const std::string& url, const std::string& localPath);

// 下载模型文件并保存到对应格式的文件夹
std::string downloadModelFile(const std::string& url, ModelFormat format);

// 下载预览图片，与模型文件放在同一目录
std::string downloadPreviewImage(const std::string& url, const std::string& baseFileName);

// 批量下载模型文件和预览图片
Json::Value downloadModelFiles(const Json::Value& modelList, const Json::Value& previewImages);
