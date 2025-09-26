#include "tx_ai3d.h"
#include "config.h"
#include "db_utils.h"
#include "concurrent_downloader.h"

#include <tencentcloud/ai3d/v20250513/Ai3dClient.h>
#include <tencentcloud/ai3d/v20250513/model/SubmitHunyuanTo3DJobRequest.h>
#include <tencentcloud/ai3d/v20250513/model/SubmitHunyuanTo3DJobResponse.h>
#include <tencentcloud/ai3d/v20250513/model/QueryHunyuanTo3DJobRequest.h>
#include <tencentcloud/ai3d/v20250513/model/QueryHunyuanTo3DJobResponse.h>
#include <tencentcloud/core/Credential.h>
#include <tencentcloud/core/profile/ClientProfile.h>
#include <tencentcloud/core/profile/HttpProfile.h>
#include <iostream>
#include <stdexcept>
#include <jsoncpp/json/json.h>
#include <sstream>
#include <algorithm>
#include <filesystem>

using namespace TencentCloud;
using namespace TencentCloud::Ai3d::V20250513;
using namespace TencentCloud::Ai3d::V20250513::Model;

// 辅助函数：下载文件并保存到数据库
static void downloadAndSaveFiles(const std::string& jobId, 
                                const Json::Value& modelList, 
                                const Json::Value& previewList,
                                Json::Value& taskInfo)
{
    try {
        // 使用并发下载器提升下载效率
        Json::Value downloadResult = getOptimizedModelDownloader().downloadModelFilesConcurrent(modelList, previewList);
        if (downloadResult["success"].asBool()) {
            // 将本地文件URL保存到数据库
            Json::Value localModels = downloadResult["modelList"];
            Json::Value localPreviews = downloadResult["previewImages"];
            
            // 构建本地文件URL字符串
            std::string localFileUrls;
            std::string localPreviewUrls;
            
            for (const auto& model : localModels) {
                if (!localFileUrls.empty()) localFileUrls += ",";
                localFileUrls += model["fileUrl"].asString();
            }
            
            for (const auto& preview : localPreviews) {
                if (!localPreviewUrls.empty()) localPreviewUrls += ",";
                localPreviewUrls += preview.asString();
            }
            
            // 更新数据库
            updateAi3dTaskFiles(jobId, localFileUrls, localPreviewUrls);
            
            // 更新返回结果，使用本地URL
            taskInfo["modelList"] = localModels;
            taskInfo["previewImages"] = localPreviews;
        }
    } catch (const std::exception& e) {
        std::cerr << "下载文件失败: " << e.what() << std::endl;
        // 下载失败不影响原有逻辑，继续返回腾讯云的URL
    }
}

// 统一的本地文件校验与补齐
static void ensureLocalFiles(const std::string& jobId,
                            const Json::Value& modelList,
                            const Json::Value& previewList,
                            Json::Value& taskInfo)
{
    // 先把来自腾讯云的结果放入 taskInfo，作为回退
    taskInfo["modelList"] = modelList;
    taskInfo["previewImages"] = previewList;

    // 检查数据库中是否已经存在本地文件URL
    Json::Value existingFiles = getTaskFileInfo(jobId);
    if (existingFiles.get("found", false).asBool()) {
        std::string existingFileUrls = existingFiles.get("fileurl", "").asString();
        std::string existingPreviewUrls = existingFiles.get("previewImages", "").asString();

        if (!existingFileUrls.empty()) {
            std::cout << "任务 " << jobId << " 已存在本地文件，进行校验" << std::endl;

            // 解析现有的本地文件URL并设置到返回结果中
            Json::Value localModels(Json::arrayValue);
            Json::Value localPreviews(Json::arrayValue);

            // 分割文件URL
            std::stringstream ss1(existingFileUrls);
            std::string item;
            while (std::getline(ss1, item, ',')) {
                if (!item.empty()) {
                    Json::Value modelItem;
                    modelItem["fileUrl"] = item;
                    // 从URL中提取文件格式
                    std::string ext = item.substr(item.find_last_of(".") + 1);
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    modelItem["fileFormat"] = ext;
                    localModels.append(modelItem);
                }
            }

            // 分割预览图片URL
            std::stringstream ss2(existingPreviewUrls);
            while (std::getline(ss2, item, ',')) {
                if (!item.empty()) {
                    localPreviews.append(item);
                }
            }

            // 校验本地物理文件是否存在，不存在则触发下载补齐
            bool needRedownload = false;
            for (const auto& m : localModels) {
                std::string url = m["fileUrl"].asString();
                std::string rel = url;
                if (rel.rfind(MODEL_URL_BASE_PATH, 0) == 0) {
                    rel = rel.substr(std::string(MODEL_URL_BASE_PATH).size());
                }
                if (!rel.empty() && rel[0] != '/') rel = std::string("/") + rel;
                std::string absPath = std::string(MODEL_FS_BASE_DIR) + rel;
                if (!std::filesystem::exists(absPath)) { needRedownload = true; break; }
            }
            if (!needRedownload) {
                for (const auto& p : localPreviews) {
                    std::string url = p.asString();
                    std::string rel = url;
                    if (rel.rfind(MODEL_URL_BASE_PATH, 0) == 0) {
                        rel = rel.substr(std::string(MODEL_URL_BASE_PATH).size());
                    }
                    if (!rel.empty() && rel[0] != '/') rel = std::string("/") + rel;
                    std::string absPath = std::string(MODEL_FS_BASE_DIR) + rel;
                    if (!std::filesystem::exists(absPath)) { needRedownload = true; break; }
                }
            }

            if (needRedownload) {
                std::cout << "任务 " << jobId << " 本地文件缺失，开始补齐下载" << std::endl;
                downloadAndSaveFiles(jobId, modelList, previewList, taskInfo);
            } else {
                // 使用现有的本地URL
                taskInfo["modelList"] = localModels;
                taskInfo["previewImages"] = localPreviews;
            }
        } else {
            // 数据库中不存在本地文件URL，进行下载
            std::cout << "任务 " << jobId << " 不存在本地文件，开始下载" << std::endl;
            downloadAndSaveFiles(jobId, modelList, previewList, taskInfo);
        }
    } else {
        // 数据库中不存在任务记录，进行下载
        std::cout << "任务 " << jobId << " 不存在数据库记录，开始下载" << std::endl;
        downloadAndSaveFiles(jobId, modelList, previewList, taskInfo);
    }
}

static Ai3dClient buildClient()
{
    Credential cred(TENCENTCLOUD_SECRET_ID, TENCENTCLOUD_SECRET_KEY);

    HttpProfile httpProfile;
    httpProfile.SetEndpoint("ai3d.tencentcloudapi.com");

    ClientProfile clientProfile;
    clientProfile.SetHttpProfile(httpProfile);

    Ai3dClient client(cred, "", clientProfile);
    client.SetRegion(TENCENTCLOUD_REGION);
    return client;
}

Json::Value submitHunyuanTo3DJob(const std::string &prompt,
                                 const std::string &imageBase64,
                                 const std::string &resultFormat)
{
    Ai3dClient client = buildClient();

    SubmitHunyuanTo3DJobRequest req;
    req.SetPrompt(prompt);
    if (!imageBase64.empty())
    {
        req.SetImageBase64(imageBase64);
    }
    if (!resultFormat.empty())
    {
        req.SetResultFormat(resultFormat);
    }
    // 根据 SDK，布尔可选项应通过 Set 方法设置，这里仅示例不默认开启
    req.SetEnablePBR(true);

    auto outcome = client.SubmitHunyuanTo3DJob(req);
    if (!outcome.IsSuccess())
    {
        throw std::runtime_error(outcome.GetError().PrintAll());
    }
    SubmitHunyuanTo3DJobResponse resp = outcome.GetResult();

    Json::Value ret;
    ret["requestId"] = resp.GetRequestId();
    ret["jobId"] = resp.GetJobId();
    return ret;
}


Json::Value submitHunyuanTo3DJobPro(const std::string &prompt,
                                    const std::string &imageBase64,
                                    const std::string &resultFormat)
{
    Ai3dClient client = buildClient();

    SubmitHunyuanTo3DProJobRequest req;
    req.SetPrompt(prompt);
    if (!imageBase64.empty())
    {
        req.SetImageBase64(imageBase64);
    }
    req.SetGenerateType("Normal");
    //req.SetMultiViewImages()
    // if (!resultFormat.empty())
    // {
    //     //req.SetResultFormat(resultFormat);
    // }
    // 根据 SDK，布尔可选项应通过 Set 方法设置，这里仅示例不默认开启
    req.SetEnablePBR(true);

    auto outcome = client.SubmitHunyuanTo3DProJob(req);
    if (!outcome.IsSuccess())
    {
        throw std::runtime_error(outcome.GetError().PrintAll());
    }
    SubmitHunyuanTo3DProJobResponse resp = outcome.GetResult();

    Json::Value ret;
    ret["requestId"] = resp.GetRequestId();
    ret["jobId"] = resp.GetJobId();
    return ret;
}

Json::Value submitHunyuanTo3DJobRapid(const std::string &prompt,
                                 const std::string &imageBase64,
                                 const std::string &resultFormat)
{
    Ai3dClient client = buildClient();

    SubmitHunyuanTo3DRapidJobRequest req;
    req.SetPrompt(prompt);
    if (!imageBase64.empty())
    {
        req.SetImageBase64(imageBase64);
    }
    if (!resultFormat.empty())
    {
        req.SetResultFormat(resultFormat);
    }
    // 根据 SDK，布尔可选项应通过 Set 方法设置，这里仅示例不默认开启
    req.SetEnablePBR(true);
    

    auto outcome = client.SubmitHunyuanTo3DRapidJob(req);
    if (!outcome.IsSuccess())
    {
        throw std::runtime_error(outcome.GetError().PrintAll());
    }
    SubmitHunyuanTo3DRapidJobResponse resp = outcome.GetResult();

    Json::Value ret;
    ret["requestId"] = resp.GetRequestId();
    ret["jobId"] = resp.GetJobId();
    return ret;
}

Json::Value queryTaskStatusFromTx(const std::string &jobId)
{
    Json::Value taskInfo;
    taskInfo["jobId"] = jobId;
    try
    {
        Ai3dClient client = buildClient();
        QueryHunyuanTo3DJobRequest req;
        req.SetJobId(jobId);
        auto outcome = client.QueryHunyuanTo3DJob(req);
        if (!outcome.IsSuccess())
        {
            std::string errMsg = outcome.GetError().PrintAll();
            taskInfo["status"] = "QUERY_FAILED";
            taskInfo["errorMsg"] = errMsg;
            return taskInfo;
        }
        QueryHunyuanTo3DJobResponse resp = outcome.GetResult();

        taskInfo["requestId"] = resp.GetRequestId();
        taskInfo["status"] = resp.GetStatus();
        //std::cout <<"CommJob: "<< resp.GetStatus()<<std::endl;
        if (resp.GetStatus() == "DONE")
        {
            auto fileList = resp.GetResultFile3Ds();
            Json::Value modelList;
            Json::Value previewList;
            for (const auto &file : fileList)
            {
                Json::Value modelItem;
                modelItem["fileUrl"] = file.GetUrl();
                modelItem["fileFormat"] = file.GetType();
                modelList.append(modelItem);
                previewList.append(file.GetPreviewImageUrl());
            }
            ensureLocalFiles(jobId, modelList, previewList, taskInfo);
        }
        else if (resp.GetStatus() == "FAIL")
        {
            taskInfo["errorMsg"] = resp.GetErrorMessage();
        }
        else if(resp.GetStatus() == "WAIT")
        {
            taskInfo["Msg"] = "任务等待中";
        }
        else if(resp.GetStatus() == "RUN")
        {
            taskInfo["Msg"] = "任务进行中";
        }
    }
    catch (const std::exception &e)
    {
        taskInfo["status"] = "QUERY_EXCEPTION";
        taskInfo["errorMsg"] = e.what();
    }
    return taskInfo;
} 

Json::Value queryTaskStatusFromTxPro(const std::string &jobId)
{
    Json::Value taskInfo;
    taskInfo["jobId"] = jobId;
    try
    {
        Ai3dClient client = buildClient();
        QueryHunyuanTo3DProJobRequest req;
        req.SetJobId(jobId);
        auto outcome = client.QueryHunyuanTo3DProJob(req);
        if (!outcome.IsSuccess())
        {
            std::string errMsg = outcome.GetError().PrintAll();
            taskInfo["status"] = "QUERY_FAILED";
            taskInfo["errorMsg"] = errMsg;
            return taskInfo;
        }
        QueryHunyuanTo3DProJobResponse resp = outcome.GetResult();
        taskInfo["requestId"] = resp.GetRequestId();
        taskInfo["status"] = resp.GetStatus();
        std::cout << "ProJob: " << resp.GetStatus() <<std::endl; 
        if (resp.GetStatus() == "DONE")
        {
            auto fileList = resp.GetResultFile3Ds();
            Json::Value modelList;
            Json::Value previewList;
            for (const auto &file : fileList)
            {
                
                Json::Value modelItem;
                modelItem["fileUrl"] = file.GetUrl();
                modelItem["fileFormat"] = file.GetType();
                modelItem["requestId"] = file.GetRequestId();
                modelList.append(modelItem);
                previewList.append(file.GetPreviewImageUrl());

            }
            taskInfo["modelList"] = modelList;
            taskInfo["previewImages"] = previewList;
            
            // 检查数据库中是否已经存在本地文件URL
            Json::Value existingFiles = getTaskFileInfo(jobId);
            ensureLocalFiles(jobId, modelList, previewList, taskInfo);
        }
        else if (resp.GetStatus() == "FAIL")
        {
            taskInfo["errorMsg"] = resp.GetErrorMessage();
        }else if(resp.GetStatus() == "WAIT")
        {
            taskInfo["Msg"] = "任务等待中";
        }
        else if(resp.GetStatus() == "RUN")
        {
            taskInfo["Msg"] = "任务进行中";
        }
    }
    catch (const std::exception &e)
    {
        taskInfo["status"] = "QUERY_EXCEPTION";
        taskInfo["errorMsg"] = e.what();
    }
    return taskInfo;
} 

Json::Value queryTaskStatusFromTxRapid(const std::string &jobId)
{
    Json::Value taskInfo;
    taskInfo["jobId"] = jobId;
    try
    {
        Ai3dClient client = buildClient();
        QueryHunyuanTo3DRapidJobRequest req;
        req.SetJobId(jobId);
        auto outcome = client.QueryHunyuanTo3DRapidJob(req);
        if (!outcome.IsSuccess())
        {
            std::string errMsg = outcome.GetError().PrintAll();
            taskInfo["status"] = "QUERY_FAILED";
            taskInfo["errorMsg"] = errMsg;
            return taskInfo;
        }
        QueryHunyuanTo3DRapidJobResponse resp = outcome.GetResult();
        taskInfo["requestId"] = resp.GetRequestId();
        taskInfo["status"] = resp.GetStatus();
        //std::cout << "rapidJob: " << resp.GetStatus() <<std::endl; 
        if (resp.GetStatus() == "DONE")
        {
            auto fileList = resp.GetResultFile3Ds();
            Json::Value modelList;
            Json::Value previewList;
            for (const auto &file : fileList)
            {
                Json::Value modelItem;
                modelItem["fileUrl"] = file.GetUrl();
                modelItem["fileFormat"] = file.GetType();
                modelItem["requestId"] = file.GetRequestId();
                modelList.append(modelItem);
                previewList.append(file.GetPreviewImageUrl());
            }
            ensureLocalFiles(jobId, modelList, previewList, taskInfo);
        }
        else if (resp.GetStatus() == "FAIL")
        {
            taskInfo["errorMsg"] = resp.GetErrorMessage();
        }else if(resp.GetStatus() == "WAIT")
        {
            taskInfo["Msg"] = "任务等待中";
        }
        else if(resp.GetStatus() == "RUN")
        {
            taskInfo["Msg"] = "任务进行中";
        }
    }
    catch (const std::exception &e)
    {
        taskInfo["status"] = "QUERY_EXCEPTION";
        taskInfo["errorMsg"] = e.what();
    }
    return taskInfo;
} 