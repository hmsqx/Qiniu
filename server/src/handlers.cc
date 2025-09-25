#include "handlers.h"

#include <jsoncpp/json/json.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include "tx_ai3d.h"
#include "db_utils.h"
#include "auth.h"
#include "config.h"
#include <algorithm>

void handleGetModel(const httplib::Request &req, httplib::Response &res)
{
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream reqBodyStream(req.body);

    if (!Json::parseFromStream(reader, reqBodyStream, &root, &errors))
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 400;
        errorResponse["message"] = std::string("无效的JSON格式: ") + errors;
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }

    std::string prompt = root.get("Prompt", "").asString();
    std::string version = root.get("Action", "").asString();
    std::string imageBase64 = root.get("ImageBase64", "").asString();
    std::string resultFormat = root.get("ResultFormat", "").asString();
    //增加UserId
    std::string userId = root.get("UserId", "").asString();

    //转化
    if(version =="SubmitHunyuanTo3DJob")version ="comm";
    else if(version == "SubmitHunyuanTo3DProJob") version = "pro";
    else version = "rapid";

    try
    {
        // 鉴权：从 Session-Token 读取当前用户信息，并尝试消费一次 token
        std::string sessionToken;
        if (req.has_header("Session-Token"))
        {
            sessionToken = req.get_header_value("Session-Token");
        }
        Json::Value userInfo;
        if (!sessionToken.empty())
        {
            userInfo = getUserInfoBySessionToken(sessionToken);
        }
        std::string currentUserId = root.get("UserId", "").asString();
        if (currentUserId.empty())
        {
            currentUserId = userInfo.get("userId", "").asString();
        }
        if (currentUserId.empty())
        {
            Json::Value errorResponse;
            errorResponse["status"] = "error";
            errorResponse["code"] = 401;
            errorResponse["message"] = "未登录或缺少用户信息";
            Json::StreamWriterBuilder writer;
            res.status = 401;
            res.set_content(Json::writeString(writer, errorResponse), "application/json");
            return;
        }

        // token_count 扣减（小于等于0则禁止调用）
        if (!tryConsumeUserTokenOnce(currentUserId))
        {
            Json::Value errorResponse;
            errorResponse["status"] = "error";
            errorResponse["code"] = 402;
            errorResponse["message"] = "余额不足，无法调用";
            Json::StreamWriterBuilder writer;
            res.status = 402;
            res.set_content(Json::writeString(writer, errorResponse), "application/json");
            return;
        }
        //version分流
        Json::Value submitResp;
        if(version == "comm")
        submitResp = submitHunyuanTo3DJob(prompt, imageBase64, resultFormat);
        else if(version == "pro")
        submitResp = submitHunyuanTo3DJobPro(prompt, imageBase64, resultFormat);
        else
        submitResp = submitHunyuanTo3DJobRapid(prompt, imageBase64, resultFormat);

        // 将任务写入数据库
        std::string jobId = submitResp.get("jobId", "").asString();
        std::string requestId = submitResp.get("requestId", "").asString();
        bool ok = insertAi3dTask(currentUserId, jobId, requestId, prompt, resultFormat, "QUEUING", version);
        if (!ok)
        {
            Json::Value errorResponse;
            errorResponse["status"] = "error";
            errorResponse["code"] = 500;
            errorResponse["message"] = "数据库写入失败";
            Json::StreamWriterBuilder writer;
            res.status = 500;
            res.set_content(Json::writeString(writer, errorResponse), "application/json");
            return;
        }
        //启用定时器，在任务完成前，每隔一段时间更新数据库中的任务状态。
        std::thread([jobId,version]() {
            using namespace std::chrono;
            auto start = steady_clock::now();
            while (duration_cast<seconds>(steady_clock::now() - start).count() < AI3D_POLL_TIMEOUT_SECONDS)
            {
                
                Json::Value taskInfo;
                if(version == "comm")
                taskInfo = queryTaskStatusFromTx(jobId);
                else if(version == "pro")
                taskInfo = queryTaskStatusFromTxPro(jobId);
                else if(version == "rapid")
                taskInfo = queryTaskStatusFromTxRapid(jobId);
                std::cout << "定时器任务执行中jobId: " << jobId <<" status:" <<taskInfo.get("status","")<< std::endl;  
                std::string status = taskInfo.get("status", "").asString();
                if (status == "DONE")
                {
                    updateAi3dTaskStatus(jobId, "SUCCEED");
                    break;
                }
                else if (status == "FAIL" || status == "QUERY_FAILED" || status == "QUERY_EXCEPTION")
                {
                    std::string err = taskInfo.get("errorMsg", "").asString();
                    updateAi3dTaskError(jobId, err);
                    break;
                }
                else if (status == "RUN" || status == "WAIT")
                {
                    updateAi3dTaskStatus(jobId, status);
                }
                std::this_thread::sleep_for(seconds(AI3D_POLL_INTERVAL_SECONDS));
            }
        }).detach();

        Json::Value response;
        response["status"] = "success";
        response["code"] = 200;
        response["message"] = "上传成功";
        response["data"] = submitResp; // 包含 jobId/requestId
        Json::StreamWriterBuilder writer;
        res.set_content(Json::writeString(writer, response), "application/json");
    }
    catch (const std::exception &e)
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 500;
        errorResponse["message"] = "提交任务失败";
        errorResponse["data"]["errorDetail"] = e.what();
        Json::StreamWriterBuilder writer;
        res.status = 500;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
    }
}

void handleQueryJobsByPage(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        int pageNum = 1;
        int pageSize = 10;
        std::string errors;
        bool isParamValid = true;
        std::string userId = "";
        if(req.has_param("UserId"))
        userId = req.get_param_value("UserId");
        std::string version = req.has_param("Version") ? req.get_param_value("version") : "comm";
        if (req.has_param("PageNum"))
        {
            try
            {
                pageNum = std::stoi(req.get_param_value("PageNum"));
                if (pageNum < 1)
                {
                    errors += "pageNum≥1；";
                    isParamValid = false;
                }
            }
            catch (...)
            {
                errors += "pageNum需为整数；";
                isParamValid = false;
            }
        }

        if (req.has_param("PageSize"))
        {
            try
            {
                pageSize = std::stoi(req.get_param_value("PageSize"));
                if (pageSize < 1 || pageSize > 50)
                {
                    errors += "1≤pageSize≤50；";
                    isParamValid = false;
                }
            }
            catch (...)
            {
                errors += "pageSize需为整数；";
                isParamValid = false;
            }
        }

        if (!isParamValid)
        {
            Json::Value errorResp;
            errorResp["status"] = "error";
            errorResp["code"] = 400;
            errorResp["message"] = std::string("参数错误：") + errors;
            Json::StreamWriterBuilder writer;
            res.status = 400;
            res.set_content(Json::writeString(writer, errorResp), "application/json");
            return;
        }

        std::string currentUserId = userId; // TODO: 从鉴权中解析
        auto result = getTaskIdsByMySQLCAPI(currentUserId, pageNum, pageSize, version);
        int totalCount = result.first;
        const auto &taskIdList = result.second;

        int totalPage = (totalCount + pageSize - 1) / pageSize;
        if (totalPage <= 0)
        {
            totalPage = 0;
        }
        if (pageNum > totalPage && totalPage > 0)
        {
            pageNum = totalPage;
        }

        Json::Value currentPageData;
        for (const auto &job : taskIdList)
        {
            // 首先从数据库获取完整的任务信息
            Json::Value dbInfo = getTaskCompleteInfo(job.first);
            
            // 然后从腾讯云获取最新的任务状态
            Json::Value taskInfo;
            std::cout <<"UserId: "<< userId<<" jobId: "<<job.first<<" job version:" <<job.second <<std::endl;
            if(job.second == "rapid")
            taskInfo = queryTaskStatusFromTxRapid(job.first);
            else if(job.second == "pro")
            taskInfo = queryTaskStatusFromTxPro(job.first);
            else
            taskInfo = queryTaskStatusFromTx(job.first);

            // 合并数据库信息和腾讯云状态信息
            if (dbInfo.get("found", false).asBool()) {
                taskInfo["fileurl"] = dbInfo["fileurl"];
                if(!dbInfo["previewImages"].empty())
                taskInfo["previewImages"] = dbInfo["previewImages"];
                taskInfo["Isprivate"] = dbInfo["Isprivate"];
                taskInfo["downloadCount"] = dbInfo["downloadCount"];
                taskInfo["like"] = dbInfo["like"];
                taskInfo["createTime"] = dbInfo["createTime"];
            }

            currentPageData.append(taskInfo);
        }

        Json::Value successResp;
        successResp["status"] = "success";
        successResp["code"] = 200;
        successResp["message"] = "分页查询任务成功";

        Json::Value pageInfo;
        pageInfo["pageNum"] = pageNum;
        pageInfo["pageSize"] = pageSize;
        pageInfo["totalCount"] = totalCount;
        pageInfo["totalPage"] = totalPage;

        successResp["data"]["pageInfo"] = pageInfo;
        successResp["data"]["taskList"] = currentPageData;

        Json::StreamWriterBuilder writer;
        res.status = 200;
        res.set_content(Json::writeString(writer, successResp), "application/json");
    }
    catch (const std::exception &e)
    {
        Json::Value errorResp;
        errorResp["status"] = "error";
        errorResp["code"] = 500;
        errorResp["message"] = "服务器内部错误";
        errorResp["data"]["errorDetail"] = e.what();
        Json::StreamWriterBuilder writer;
        res.status = 500;
        res.set_content(Json::writeString(writer, errorResp), "application/json");
    }
}

void handleRegister(const httplib::Request &req, httplib::Response &res)
{
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream reqBodyStream(req.body);
    if (!Json::parseFromStream(reader, reqBodyStream, &root, &errors))
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 400;
        errorResponse["message"] = std::string("无效的JSON格式: ") + errors;
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }

    std::string username = root.get("username", "").asString();
    std::string email = root.get("email", "").asString();
    std::string password = root.get("password", "").asString();

    if (username.empty() || email.empty() || password.size() < 6)
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 400;
        errorResponse["message"] = "参数错误：username/email 必填，password 至少6位";
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }

    Json::Value result = registerUser(username, email, password);
    Json::StreamWriterBuilder writer;
    if (result.get("status", "error").asString() == "success")
    {
        res.status = 200;
    }
    else
    {
        int code = result.get("code", 500).asInt();
        res.status = (code >= 400 && code < 600) ? code : 500;
    }
    res.set_content(Json::writeString(writer, result), "application/json");
}

void handleLogin(const httplib::Request &req, httplib::Response &res)
{
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream reqBodyStream(req.body);
    if (!Json::parseFromStream(reader, reqBodyStream, &root, &errors))
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 400;
        errorResponse["message"] = std::string("无效的JSON格式: ") + errors;
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }

    std::string usernameOrEmail = root.get("usernameOrEmail", "").asString();
    std::string password = root.get("password", "").asString();

    if (usernameOrEmail.empty() || password.empty())
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 400;
        errorResponse["message"] = "参数错误：usernameOrEmail/password 必填";
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }

    Json::Value result = loginUser(usernameOrEmail, password);
    Json::StreamWriterBuilder writer;
    if (result.get("status", "error").asString() == "success")
    {
        res.status = 200;
    }
    else
    {
        int code = result.get("code", 500).asInt();
        res.status = (code >= 400 && code < 600) ? code : 500;
    }
    res.set_content(Json::writeString(writer, result), "application/json");
} 

void handleMe(const httplib::Request &req, httplib::Response &res)
{
    std::string sessionToken;
    if (req.has_header("Session-Token"))
    {
        sessionToken = req.get_header_value("Session-Token");
    }
    if (sessionToken.empty())
    {
        Json::Value errorResp;
        errorResp["status"] = "error";
        errorResp["code"] = 401;
        errorResp["message"] = "缺少 Session-Token";
        Json::StreamWriterBuilder writer;
        res.status = 401;
        res.set_content(Json::writeString(writer, errorResp), "application/json");
        return;
    }

    Json::Value info = getUserInfoBySessionToken(sessionToken);
    if (info.isNull() || info.get("userId", "").asString().empty())
    {
        Json::Value errorResp;
        errorResp["status"] = "error";
        errorResp["code"] = 401;
        errorResp["message"] = "无效或过期的会话";
        Json::StreamWriterBuilder writer;
        res.status = 401;
        res.set_content(Json::writeString(writer, errorResp), "application/json");
        return;
    }

    Json::Value resp;
    resp["status"] = "success";
    resp["code"] = 200;
    resp["message"] = "OK";
    resp["data"]["userId"] = info.get("userId", "").asString();
    resp["data"]["username"] = info.get("username", "").asString();
    resp["data"]["role"] = info.get("role", "").asString();
    resp["data"]["token_count"] = info.get("token_count", 0).asInt();
    Json::StreamWriterBuilder writer;
    res.status = 200;
    res.set_content(Json::writeString(writer, resp), "application/json");
}

void handleDownloadModel(const httplib::Request &req, httplib::Response &res)
{
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream reqBodyStream(req.body);
    if (!Json::parseFromStream(reader, reqBodyStream, &root, &errors))
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 400;
        errorResponse["message"] = std::string("无效的JSON格式: ") + errors;
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }
    std::string jobId = root.get("jobId", "").asString();
    if (jobId.empty())
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 400;
        errorResponse["message"] = "jobId 必填";
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }
    if (!incrementModelDownloadCount(jobId))
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 500;
        errorResponse["message"] = "下载计数更新失败";
        Json::StreamWriterBuilder writer;
        res.status = 500;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }
    Json::Value ok;
    ok["status"] = "success";
    ok["code"] = 200;
    ok["message"] = "下载计数+1";
    Json::StreamWriterBuilder writer;
    res.status = 200;
    res.set_content(Json::writeString(writer, ok), "application/json");
}

void handleLikeModel(const httplib::Request &req, httplib::Response &res)
{
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream reqBodyStream(req.body);
    if (!Json::parseFromStream(reader, reqBodyStream, &root, &errors))
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 400;
        errorResponse["message"] = std::string("无效的JSON格式: ") + errors;
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }
    std::string jobId = root.get("jobId", "").asString();
    if (jobId.empty())
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 400;
        errorResponse["message"] = "jobId 必填";
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }
    if (!incrementModelLikeCount(jobId))
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 500;
        errorResponse["message"] = "收藏计数更新失败";
        Json::StreamWriterBuilder writer;
        res.status = 500;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }
    Json::Value ok;
    ok["status"] = "success";
    ok["code"] = 200;
    ok["message"] = "收藏计数+1";
    Json::StreamWriterBuilder writer;
    res.status = 200;
    res.set_content(Json::writeString(writer, ok), "application/json");
}

void handleShowModel(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        int pageNum = 1;
        int pageSize = 10;
        bool isPrivate = false;
        std::string errors;
        bool isParamValid = true;

        if (req.has_param("PageNum"))
        {
            try { pageNum = std::stoi(req.get_param_value("PageNum")); if (pageNum < 1) { errors += "pageNum≥1；"; isParamValid = false; } }
            catch (...) { errors += "pageNum需为整数；"; isParamValid = false; }
        }
        if (req.has_param("PageSize"))
        {
            try { pageSize = std::stoi(req.get_param_value("PageSize")); if (pageSize < 1 || pageSize > 50) { errors += "1≤pageSize≤50；"; isParamValid = false; } }
            catch (...) { errors += "pageSize需为整数；"; isParamValid = false; }
        }
        if (req.has_param("Isprivate"))
        {
            std::string v = req.get_param_value("Isprivate");
            std::transform(v.begin(), v.end(), v.begin(), ::tolower);
            isPrivate = (v == "1" || v == "true");
        }

        if (!isParamValid)
        {
            Json::Value errorResp;
            errorResp["status"] = "error";
            errorResp["code"] = 400;
            errorResp["message"] = std::string("参数错误：") + errors;
            Json::StreamWriterBuilder writer;
            res.status = 400;
            res.set_content(Json::writeString(writer, errorResp), "application/json");
            return;
        }

        auto pair = queryModelsByPrivacy(isPrivate, pageNum, pageSize);
        int totalCount = pair.first;
        const Json::Value &items = pair.second;
        int totalPage = (totalCount + pageSize - 1) / pageSize;
        if (totalPage <= 0) totalPage = 0;
        if (pageNum > totalPage && totalPage > 0) pageNum = totalPage;

        Json::Value successResp;
        successResp["status"] = "success";
        successResp["code"] = 200;
        successResp["message"] = "查询成功";
        Json::Value pageInfo;
        pageInfo["pageNum"] = pageNum;
        pageInfo["pageSize"] = pageSize;
        pageInfo["totalCount"] = totalCount;
        pageInfo["totalPage"] = totalPage;
        successResp["data"]["pageInfo"] = pageInfo;
        successResp["data"]["list"] = items;

        Json::StreamWriterBuilder writer;
        res.status = 200;
        res.set_content(Json::writeString(writer, successResp), "application/json");
    }
    catch (const std::exception &e)
    {
        Json::Value errorResp;
        errorResp["status"] = "error";
        errorResp["code"] = 500;
        errorResp["message"] = "服务器内部错误";
        errorResp["data"]["errorDetail"] = e.what();
        Json::StreamWriterBuilder writer;
        res.status = 500;
        res.set_content(Json::writeString(writer, errorResp), "application/json");
    }
}

void handleIncrTokenCount(const httplib::Request &req, httplib::Response &res)
{
    std::string sessionToken;
    if (req.has_header("Session-Token")) {
        sessionToken = req.get_header_value("Session-Token");
    }
    if (sessionToken.empty()) {
        Json::Value errorResp;
        errorResp["status"] = "error";
        errorResp["code"] = 401;
        errorResp["message"] = "缺少 Session-Token";
        Json::StreamWriterBuilder writer;
        res.status = 401;
        res.set_content(Json::writeString(writer, errorResp), "application/json");
        return;
    }

    Json::Value info = getUserInfoBySessionToken(sessionToken);
    std::string userId = info.get("userId", "").asString();
    if (userId.empty()) {
        Json::Value errorResp;
        errorResp["status"] = "error";
        errorResp["code"] = 401;
        errorResp["message"] = "无效或过期的会话";
        Json::StreamWriterBuilder writer;
        res.status = 401;
        res.set_content(Json::writeString(writer, errorResp), "application/json");
        return;
    }
    int delta = 1;
    delta = stoi(req.get_header_value("delta"));
    bool ok = updateUserTokenCount(userId, delta);
    if (!ok) {
        Json::Value errorResp;
        errorResp["status"] = "error";
        errorResp["code"] = 500;
        errorResp["message"] = "token_count 增加失败";
        Json::StreamWriterBuilder writer;
        res.status = 500;
        res.set_content(Json::writeString(writer, errorResp), "application/json");
        return;
    }

    Json::Value respJson;
    respJson["status"] = "success";
    respJson["code"] = 200;
    respJson["message"] = "token_count +1";
    Json::StreamWriterBuilder writer;
    res.status = 200;
    res.set_content(Json::writeString(writer, respJson), "application/json");
}

void handleToggleJobIsPrivate(const httplib::Request &req, httplib::Response &res)
{
    Json::Value respJson;
    Json::Reader reader;
    Json::Value body;
    if (!reader.parse(req.body, body) || !body.isMember("jobId")) {
        respJson["status"] = "error";
        respJson["code"] = 400;
        respJson["message"] = "缺少 jobId 参数";
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, respJson), "application/json");
        return;
    }
    std::string jobId = body["jobId"].asString();
    bool ok = toggleJobIsPrivate(jobId);
    if (!ok) {
        respJson["status"] = "error";
        respJson["code"] = 500;
        respJson["message"] = "修改失败";
        Json::StreamWriterBuilder writer;
        res.status = 500;
        res.set_content(Json::writeString(writer, respJson), "application/json");
        return;
    }
    respJson["status"] = "success";
    respJson["code"] = 200;
    respJson["message"] = "Isprivate 已取反";
    Json::StreamWriterBuilder writer;
    res.status = 200;
    res.set_content(Json::writeString(writer, respJson), "application/json");
}

void handleGetTaskFiles(const httplib::Request &req, httplib::Response &res)
{
    std::string jobId;
    
    // 从URL参数或请求体中获取jobId
    if (req.has_param("jobId")) {
        jobId = req.get_param_value("jobId");
    } else {
        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errors;
        std::istringstream reqBodyStream(req.body);
        
        if (Json::parseFromStream(reader, reqBodyStream, &root, &errors)) {
            jobId = root.get("jobId", "").asString();
        }
    }
    
    if (jobId.empty()) {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 400;
        errorResponse["message"] = "jobId 必填";
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }
    
    Json::Value taskInfo = getTaskFileInfo(jobId);
    
    if (!taskInfo.get("found", false).asBool()) {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 404;
        errorResponse["message"] = "任务不存在";
        Json::StreamWriterBuilder writer;
        res.status = 404;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }
    
    // 解析文件URL和预览图片URL
    std::string fileUrls = taskInfo.get("fileurl", "").asString();
    std::string previewUrls = taskInfo.get("previewImages", "").asString();
    
    Json::Value fileList(Json::arrayValue);
    Json::Value previewList(Json::arrayValue);
    
    // 分割文件URL（用逗号分隔）
    if (!fileUrls.empty()) {
        std::stringstream ss(fileUrls);
        std::string item;
        while (std::getline(ss, item, ',')) {
            if (!item.empty()) {
                fileList.append(item);
            }
        }
    }
    
    // 分割预览图片URL（用逗号分隔）
    if (!previewUrls.empty()) {
        std::stringstream ss(previewUrls);
        std::string item;
        while (std::getline(ss, item, ',')) {
            if (!item.empty()) {
                previewList.append(item);
            }
        }
    }
    
    Json::Value response;
    response["status"] = "success";
    response["code"] = 200;
    response["message"] = "获取任务文件信息成功";
    response["data"]["jobId"] = taskInfo.get("jobId", "");
    response["data"]["Isprivate"] = taskInfo.get("Isprivate", "");
    response["data"]["status"] = taskInfo.get("status", "");
    response["data"]["prompt"] = taskInfo.get("prompt", "");
    response["data"]["resultFormat"] = taskInfo.get("resultFormat", "");
    response["data"]["version"] = taskInfo.get("version", "");
    response["data"]["createTime"] = taskInfo.get("createTime", "");
    response["data"]["fileList"] = fileList;
    response["data"]["previewList"] = previewList;

    
    Json::StreamWriterBuilder writer;
    res.status = 200;
    res.set_content(Json::writeString(writer, response), "application/json");
}