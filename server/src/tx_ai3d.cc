#include "tx_ai3d.h"
#include "config.h"

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

using namespace TencentCloud;
using namespace TencentCloud::Ai3d::V20250513;
using namespace TencentCloud::Ai3d::V20250513::Model;

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
        std::cout <<"CommJob: "<< resp.GetStatus()<<std::endl;
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
            taskInfo["modelList"] = modelList;
            taskInfo["previewImages"] = previewList;
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
        std::cout << "rapidJob: " << resp.GetStatus() <<std::endl; 
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