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
    std::string version = root.get("Version", "").asString();
    std::string imageBase64 = root.get("ImageBase64", "").asString();
    std::string resultFormat = root.get("ResultFormat", "").asString();
    //增加UserId
    std::string userId = root.get("userId", "").asString();

    try
    {
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
        std::string currentUserId = userId; // TODO: 替换为登录态解析
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
        std::thread([jobId]() {
            using namespace std::chrono;
            auto start = steady_clock::now();
            while (duration_cast<seconds>(steady_clock::now() - start).count() < AI3D_POLL_TIMEOUT_SECONDS)
            {
                Json::Value taskInfo = queryTaskStatusFromTx(jobId);
                std::string status = taskInfo.get("status", "").asString();
                if (status == "SUCCEED")
                {
                    updateAi3dTaskStatus(jobId, "SUCCEED");
                    break;
                }
                else if (status == "FAILED" || status == "QUERY_FAILED" || status == "QUERY_EXCEPTION")
                {
                    std::string err = taskInfo.get("errorMsg", "").asString();
                    updateAi3dTaskError(jobId, err);
                    break;
                }
                else if (status == "RUNNING" || status == "QUEUING")
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
        if(req.has_param("userId"))
        userId = req.get_param_value("userId");
        std::string version = req.has_param("version") ? req.get_param_value("version") : "comm";
        if (req.has_param("pageNum"))
        {
            try
            {
                pageNum = std::stoi(req.get_param_value("pageNum"));
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

        if (req.has_param("pageSize"))
        {
            try
            {
                pageSize = std::stoi(req.get_param_value("pageSize"));
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
            //查询分流
            Json::Value taskInfo;
            if(job.second == "comm")
            taskInfo = queryTaskStatusFromTx(job.first);
            else if(job.second == "pro")
            taskInfo = queryTaskStatusFromTxPro(job.first);
            else
            taskInfo = queryTaskStatusFromTxRapid(job.first);

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