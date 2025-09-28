#include "handlers.h"

#include <jsoncpp/json/json.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include "tx_ai3d.h"
#include "thread_pool.h"
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
    // 增加UserId
    std::string userId = root.get("UserId", "").asString();

    // 转化
    if (version == "SubmitHunyuanTo3DJob")
        version = "comm";
    else if (version == "SubmitHunyuanTo3DProJob")
        version = "pro";
    else
        version = "rapid";

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
        if (!consumeUserTokenSafely(currentUserId))
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
        // version分流
        Json::Value submitResp;
        if (version == "comm")
            submitResp = submitHunyuanTo3DJob(prompt, imageBase64, resultFormat);
        else if (version == "pro")
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
        // 启用定时器，在任务完成前，每隔一段时间更新数据库中的任务状态。
        std::thread([jobId, version]()
                    {
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
            } })
            .detach();

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
        errorResponse["message"] = std::string("提交任务失败: ") + e.what();
        errorResponse["data"]["errorDetail"] = e.what();
        Json::StreamWriterBuilder writer;
        res.status = 500;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
    }
}

void handleGetUserLike(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errors;
        std::istringstream reqBodyStream(req.body);
        Json::parseFromStream(reader, reqBodyStream, &root, &errors);

        std::string sessionToken = req.has_header("Session-Token") ? req.get_header_value("Session-Token") : "";
        Json::Value userInfo = sessionToken.empty() ? Json::Value() : getUserInfoBySessionToken(sessionToken);
        std::string userId = root.get("UserId", "").asString();
        if (userId.empty())
            userId = userInfo.get("userId", "").asString();
        std::string jobId = root.get("jobId", "").asString();
        if (userId.empty() || jobId.empty())
        {
            Json::Value err;
            err["status"] = "error";
            err["code"] = 400;
            err["message"] = "userId/jobId 必填";
            Json::StreamWriterBuilder w;
            res.status = 400;
            res.set_content(Json::writeString(w, err), "application/json");
            return;
        }

        bool liked = getUserLikeForJob(userId, jobId);
        Json::Value ok;
        ok["status"] = "success";
        ok["code"] = 200;
        ok["message"] = "OK";
        ok["data"]["liked"] = liked;
        Json::StreamWriterBuilder w;
        res.status = 200;
        res.set_content(Json::writeString(w, ok), "application/json");
    }
    catch (const std::exception &e)
    {
        Json::Value err;
        err["status"] = "error";
        err["code"] = 500;
        err["message"] = "服务器内部错误";
        err["data"]["errorDetail"] = e.what();
        Json::StreamWriterBuilder w;
        res.status = 500;
        res.set_content(Json::writeString(w, err), "application/json");
    }
}

void handleToggleUserLike(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errors;
        std::istringstream reqBodyStream(req.body);
        if (!Json::parseFromStream(reader, reqBodyStream, &root, &errors))
        {
            Json::Value err;
            err["status"] = "error";
            err["code"] = 400;
            err["message"] = std::string("无效的JSON格式: ") + errors;
            Json::StreamWriterBuilder w;
            res.status = 400;
            res.set_content(Json::writeString(w, err), "application/json");
            return;
        }

        std::string sessionToken = req.has_header("Session-Token") ? req.get_header_value("Session-Token") : "";
        Json::Value userInfo = sessionToken.empty() ? Json::Value() : getUserInfoBySessionToken(sessionToken);
        std::string userId = root.get("UserId", "").asString();
        if (userId.empty())
            userId = userInfo.get("userId", "").asString();
        std::string jobId = root.get("jobId", "").asString();
        if (userId.empty() || jobId.empty())
        {
            Json::Value err;
            err["status"] = "error";
            err["code"] = 400;
            err["message"] = "userId/jobId 必填";
            Json::StreamWriterBuilder w;
            res.status = 400;
            res.set_content(Json::writeString(w, err), "application/json");
            return;
        }

        bool newStatus = false;
        bool ok = toggleUserLikeForJob(userId, jobId, newStatus);
        if (!ok)
        {
            Json::Value err;
            err["status"] = "error";
            err["code"] = 500;
            err["message"] = "点赞状态更新失败";
            Json::StreamWriterBuilder w;
            res.status = 500;
            res.set_content(Json::writeString(w, err), "application/json");
            return;
        }

        Json::Value resp;
        resp["status"] = "success";
        resp["code"] = 200;
        resp["message"] = "OK";
        resp["data"]["liked"] = newStatus;
        Json::StreamWriterBuilder w;
        res.status = 200;
        res.set_content(Json::writeString(w, resp), "application/json");
    }
    catch (const std::exception &e)
    {
        Json::Value err;
        err["status"] = "error";
        err["code"] = 500;
        err["message"] = "服务器内部错误";
        err["data"]["errorDetail"] = e.what();
        Json::StreamWriterBuilder w;
        res.status = 500;
        res.set_content(Json::writeString(w, err), "application/json");
    }
}
// 异步处理：分页查询（合并自 optimized_handlers）
void handleQueryJobsByPageAsync(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        int pageNum = 1;
        int pageSize = 10;
        std::string errors;
        bool isParamValid = true;
        std::string userId = "";
        if (req.has_param("UserId"))
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

        auto fut = getThreadPool().enqueue([=]()
                                           {
            Json::Value out;
            try {
                auto result = getTaskIdsByMySQLCAPI(userId, pageNum, pageSize, version);
                int totalCount = result.first;
                const auto &taskIdList = result.second;

                int totalPage = (totalCount + pageSize - 1) / pageSize;
                if (totalPage <= 0) totalPage = 0;
                int finalPageNum = pageNum;
                if (finalPageNum > totalPage && totalPage > 0) finalPageNum = totalPage;

                // 并发查询每个任务的状态与DB合并，提升列表吞吐
                std::vector<std::future<Json::Value>> futures;
                futures.reserve(taskIdList.size());
                for (const auto &job : taskIdList) {
                    futures.emplace_back(getThreadPool().enqueue([job]() {
                        Json::Value merged;
                        try {
                            // 读取数据库信息
                            Json::Value dbInfo = getTaskCompleteInfo(job.first);
                            // 如果已完成，直接返回DB信息（避免外部重复查询）
                            if (dbInfo["status"].asString() == "SUCCEED" || dbInfo["status"].asString() == "DONE") {
                                return dbInfo;
                            }
                            // 远端状态查询
                            Json::Value taskInfo;
                            if (job.second == "rapid") taskInfo = queryTaskStatusFromTxRapid(job.first);
                            else if (job.second == "pro") taskInfo = queryTaskStatusFromTxPro(job.first);
                            else taskInfo = queryTaskStatusFromTx(job.first);

                            // 合并DB信息
                            if (dbInfo.get("found", false).asBool()) {
                                taskInfo["fileurl"] = dbInfo["fileurl"];
                                if (!dbInfo["previewImages"].empty()) taskInfo["previewImages"] = dbInfo["previewImages"];
                                taskInfo["Isprivate"] = dbInfo["Isprivate"];
                                taskInfo["downloadCount"] = dbInfo["downloadCount"];
                                taskInfo["like"] = dbInfo["like"];
                                taskInfo["createTime"] = dbInfo["createTime"];
                                if (!dbInfo["status"].asString().empty()) taskInfo["status"] = dbInfo["status"];
                                if (taskInfo["status"].asString() == "SUCCEED") taskInfo["errorMsg"] = "";
                            }
                            merged = taskInfo;
                        } catch (const std::exception &ex) {
                            merged["status"] = "error";
                            merged["message"] = std::string("任务查询异常: ") + ex.what();
                        }
                        return merged;
                    }));
                }

                Json::Value currentPageData(Json::arrayValue);
                for (auto &f : futures) {
                    currentPageData.append(f.get());
                }

                out["status"] = "success";
                out["code"] = 200;
                out["message"] = "分页查询任务成功";
                Json::Value pageInfo;
                pageInfo["pageNum"] = finalPageNum;
                pageInfo["pageSize"] = pageSize;
                pageInfo["totalCount"] = totalCount;
                pageInfo["totalPage"] = totalPage;
                out["data"]["pageInfo"] = pageInfo;
                out["data"]["taskList"] = currentPageData;
            } catch (const std::exception& e) {
                out["status"] = "error";
                out["code"] = 500;
                out["message"] = std::string("服务器内部错误: ") + e.what();
            }
            return out; });

        Json::Value result = fut.get();
        Json::StreamWriterBuilder writer;
        res.status = (result["status"].asString() == "error") ? result["code"].asInt() : 200;
        res.set_content(Json::writeString(writer, result), "application/json");
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
void handleQueryJobsByPage(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        int pageNum = 1;
        int pageSize = 10;
        std::string errors;
        bool isParamValid = true;
        std::string userId = "";
        if (req.has_param("UserId"))
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
            std::cout << "UserId: " << userId << " jobId: " << job.first << " job version:" << job.second << std::endl;
            if (job.second == "rapid")
                taskInfo = queryTaskStatusFromTxRapid(job.first);
            else if (job.second == "pro")
                taskInfo = queryTaskStatusFromTxPro(job.first);
            else
                taskInfo = queryTaskStatusFromTx(job.first);

            // 合并数据库信息和腾讯云状态信息
            if (dbInfo.get("found", false).asBool())
            {
                taskInfo["fileurl"] = dbInfo["fileurl"];
                if (!dbInfo["previewImages"].empty())
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

void handleLogout(const httplib::Request &req, httplib::Response &res)
{
    // 从请求头获取Session-Token
    std::string sessionToken;
    if (req.has_header("Session-Token"))
    {
        sessionToken = req.get_header_value("Session-Token");
    }

    if (sessionToken.empty())
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 400;
        errorResponse["message"] = "缺少Session-Token";
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }

    Json::Value result = logoutUser(sessionToken);
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

    // 获取当前用户信息
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

    std::string currentUserId = userInfo.get("userId", "").asString();
    if (currentUserId.empty())
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 401;
        errorResponse["message"] = "未登录或会话已过期";
        Json::StreamWriterBuilder writer;
        res.status = 401;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }

    // 使用新的下载记录函数（确保每个用户对每个模型只能计数一次）
    auto fut = getThreadPool().enqueue([currentUserId, jobId]()
                                       { return recordUserDownloadAndIncrementCount(currentUserId, jobId); });
    if (!fut.get())
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = 500;
        errorResponse["message"] = "下载记录更新失败";
        Json::StreamWriterBuilder writer;
        res.status = 500;
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }

    Json::Value ok;
    ok["status"] = "success";
    ok["code"] = 200;
    ok["message"] = "下载记录已更新";
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
    auto fut = getThreadPool().enqueue([jobId]()
                                       { return incrementModelLikeCount(jobId); });
    if (!fut.get())
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

        auto fut = getThreadPool().enqueue([=]()
                                           {
            Json::Value resp;
            auto pair = queryModelsByPrivacy(isPrivate, pageNum, pageSize);
            int totalCount = pair.first;
            const Json::Value &items = pair.second;
            int totalPage = (totalCount + pageSize - 1) / pageSize;
            if (totalPage <= 0) totalPage = 0;
            int finalPageNum = pageNum;
            if (finalPageNum > totalPage && totalPage > 0) finalPageNum = totalPage;

            resp["status"] = "success";
            resp["code"] = 200;
            resp["message"] = "查询成功";
            Json::Value pageInfo;
            pageInfo["pageNum"] = finalPageNum;
            pageInfo["pageSize"] = pageSize;
            pageInfo["totalCount"] = totalCount;
            pageInfo["totalPage"] = totalPage;
            resp["data"]["pageInfo"] = pageInfo;
            // 装配 islike 字段（存在即点赞），需要 userId
            Json::Value listWithLike(Json::arrayValue);
            std::string sessToken = req.get_header_value("Session-Token");
            Json::Value me = sessToken.empty() ? Json::Value() : getUserInfoBySessionToken(sessToken);
            std::string currentUserId = me.get("userId", "").asString();
            for (const auto &it : items) {
                Json::Value item = it;
                bool islike = false;
                if (!currentUserId.empty()) {
                    std::string jobId = it.get("jobId", "").asString();
                    if (!jobId.empty()) {
                        islike = getUserLikeForJob(currentUserId, jobId);
                    }
                }
                item["islike"] = islike;
                listWithLike.append(item);
            }
            resp["data"]["list"] = listWithLike;
            return resp; });
        Json::Value successResp = fut.get();
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
    std::string userId = info.get("userId", "").asString();
    if (userId.empty())
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
    int delta = 1;
    delta = stoi(req.get_header_value("delta"));
    auto fut = getThreadPool().enqueue([=]()
                                       { return updateUserTokenCount(userId, delta); });
    bool ok = fut.get();
    if (!ok)
    {
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
    if (!reader.parse(req.body, body) || !body.isMember("jobId"))
    {
        respJson["status"] = "error";
        respJson["code"] = 400;
        respJson["message"] = "缺少 jobId 参数";
        Json::StreamWriterBuilder writer;
        res.status = 400;
        res.set_content(Json::writeString(writer, respJson), "application/json");
        return;
    }
    std::string jobId = body["jobId"].asString();
    auto fut = getThreadPool().enqueue([=]()
                                       { return toggleJobIsPrivate(jobId); });
    bool ok = fut.get();
    if (!ok)
    {
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
    if (req.has_param("jobId"))
    {
        jobId = req.get_param_value("jobId");
    }
    else
    {
        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errors;
        std::istringstream reqBodyStream(req.body);

        if (Json::parseFromStream(reader, reqBodyStream, &root, &errors))
        {
            jobId = root.get("jobId", "").asString();
        }
    }

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

    auto fut = getThreadPool().enqueue([=]()
                                       {
        Json::Value out;
        Json::Value taskInfo = getTaskFileInfo(jobId);
        if (!taskInfo.get("found", false).asBool()) {
            out["__error__"] = 404;
            out["__message__"] = "任务不存在";
            return out;
        }
        std::string fileUrls = taskInfo.get("fileurl", "").asString();
        std::string previewUrls = taskInfo.get("previewImages", "").asString();
        Json::Value fileList(Json::arrayValue);
        Json::Value previewList(Json::arrayValue);
        if (!fileUrls.empty()) {
            std::stringstream ss(fileUrls);
            std::string item;
            while (std::getline(ss, item, ',')) {
                if (!item.empty()) fileList.append(item);
            }
        }
        if (!previewUrls.empty()) {
            std::stringstream ss(previewUrls);
            std::string item;
            while (std::getline(ss, item, ',')) {
                if (!item.empty()) previewList.append(item);
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
        return response; });
    Json::Value result = fut.get();
    if (result.isMember("__error__"))
    {
        Json::Value errorResponse;
        errorResponse["status"] = "error";
        errorResponse["code"] = result["__error__"].asInt();
        errorResponse["message"] = result["__message__"].asString();
        Json::StreamWriterBuilder writer;
        res.status = errorResponse["code"].asInt();
        res.set_content(Json::writeString(writer, errorResponse), "application/json");
        return;
    }
    Json::StreamWriterBuilder writer;
    res.status = 200;
    res.set_content(Json::writeString(writer, result), "application/json");
}

void handleIncrementViewAndGetRates(const httplib::Request& req, httplib::Response& res) {
    // 1. 解析请求JSON
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream ss(req.body);

    if (!Json::parseFromStream(reader, ss, &root, &errors)) {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 400;
        e["message"] = "无效的JSON格式: " + errors;
        Json::StreamWriterBuilder w;
        res.status = 400;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }

    // 2. 校验jobId参数
    std::string jobId = root.get("jobId", "").asString();
    if (jobId.empty()) {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 400;
        e["message"] = "jobId 必填";
        Json::StreamWriterBuilder w;
        res.status = 400;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }

    // 3. 异步执行：递增viewCount并获取统计（使用线程池）
    auto fut = getThreadPool().enqueue([jobId]() {
        Json::Value stats;
        bool success = incrementModelViewAndGetStats(jobId, stats);
        return std::make_pair(success, stats);
    });

    auto [success, stats] = fut.get();

    // 4. 处理结果并返回
    if (!success) {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 500;
        e["message"] = "浏览计数更新或查询失败";
        Json::StreamWriterBuilder w;
        res.status = 500;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }

    // 5. 构建包含下载率、收藏率和prompt的响应
    double viewCnt = stats["viewCount"].asDouble();
    double likeRate = (viewCnt <= 0) ? 0.0 : static_cast<double>(stats["likeCount"].asInt()) / viewCnt;
    double downloadRate = (viewCnt <= 0) ? 0.0 : static_cast<double>(stats["downloadCount"].asInt()) / viewCnt;

    Json::Value resp;
    resp["status"] = "success";
    resp["code"] = 200;
    resp["data"]["jobId"] = jobId;
    resp["data"]["viewCount"] = stats["viewCount"].asInt();
    resp["data"]["likeCount"] = stats["likeCount"].asInt();
    resp["data"]["downloadCount"] = stats["downloadCount"].asInt();
    resp["data"]["likeRate"] = likeRate;       // 收藏率
    resp["data"]["downloadRate"] = downloadRate; // 下载率
    resp["data"]["prompt"] = stats["prompt"].asString();     // 新增prompt字段

    Json::StreamWriterBuilder w;
    res.status = 200;
    res.set_content(Json::writeString(w, resp), "application/json");
}
static Json::Value buildRateResponse(const std::string &jobId, const std::string &rateName, int likeCnt, int downloadCnt, int viewCnt)
{
    double denom = static_cast<double>(viewCnt);
    double likeRate = (denom <= 0.0) ? 0.0 : static_cast<double>(likeCnt) / denom;
    double downloadRate = (denom <= 0.0) ? 0.0 : static_cast<double>(downloadCnt) / denom;
    Json::Value resp;
    resp["status"] = "success";
    resp["code"] = 200;
    resp["message"] = "OK";
    resp["data"]["jobId"] = jobId;
    if (rateName == "likeRate")
        resp["data"]["likeRate"] = likeRate;
    if (rateName == "downloadRate")
        resp["data"]["downloadRate"] = downloadRate;
    resp["data"]["viewCount"] = viewCnt;
    resp["data"]["like"] = likeCnt;
    resp["data"]["downloadCount"] = downloadCnt;
    return resp;
}

//deprecated
void handleGetLikeRate(const httplib::Request &req, httplib::Response &res)
{
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream ss(req.body);
    if (!Json::parseFromStream(reader, ss, &root, &errors))
    {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 400;
        e["message"] = std::string("无效的JSON格式: ") + errors;
        Json::StreamWriterBuilder w;
        res.status = 400;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }
    std::string jobId = root.get("jobId", "").asString();
    if (jobId.empty())
    {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 400;
        e["message"] = "jobId 必填";
        Json::StreamWriterBuilder w;
        res.status = 400;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }
    auto fut = getThreadPool().enqueue([jobId]()
                                       { int likeCnt=0, downCnt=0, viewCnt=0; bool ok = getTaskStats(jobId, likeCnt, downCnt, viewCnt); Json::Value out; if (!ok) { out["__error__"]=true; } else { out["like"]=likeCnt; out["downloadCount"]=downCnt; out["viewCount"]=viewCnt; } return out; });
    Json::Value resJson = fut.get();
    if (resJson.isMember("__error__"))
    {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 404;
        e["message"] = "任务不存在";
        Json::StreamWriterBuilder w;
        res.status = 404;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }
    Json::Value ok = buildRateResponse(jobId, "likeRate", resJson["like"].asInt(), resJson["downloadCount"].asInt(), resJson["viewCount"].asInt());
    Json::StreamWriterBuilder w;
    res.status = 200;
    res.set_content(Json::writeString(w, ok), "application/json");
}
//deprecated
void handleGetDownloadRate(const httplib::Request &req, httplib::Response &res)
{
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream ss(req.body);
    if (!Json::parseFromStream(reader, ss, &root, &errors))
    {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 400;
        e["message"] = std::string("无效的JSON格式: ") + errors;
        Json::StreamWriterBuilder w;
        res.status = 400;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }
    std::string jobId = root.get("jobId", "").asString();
    if (jobId.empty())
    {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 400;
        e["message"] = "jobId 必填";
        Json::StreamWriterBuilder w;
        res.status = 400;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }
    auto fut = getThreadPool().enqueue([jobId]()
                                       { int likeCnt=0, downCnt=0, viewCnt=0; bool ok = getTaskStats(jobId, likeCnt, downCnt, viewCnt); Json::Value out; if (!ok) { out["__error__"]=true; } else { out["like"]=likeCnt; out["downloadCount"]=downCnt; out["viewCount"]=viewCnt; } return out; });
    Json::Value resJson = fut.get();
    if (resJson.isMember("__error__"))
    {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 404;
        e["message"] = "任务不存在";
        Json::StreamWriterBuilder w;
        res.status = 404;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }
    Json::Value ok = buildRateResponse(jobId, "downloadRate", resJson["like"].asInt(), resJson["downloadCount"].asInt(), resJson["viewCount"].asInt());
    Json::StreamWriterBuilder w;
    res.status = 200;
    res.set_content(Json::writeString(w, ok), "application/json");
}
//deprecated
void handleGetUserGrowth(const httplib::Request &req, httplib::Response &res)
{
    // 支持 query 参数或 body 传参：start, end（ISO时间如 2025-09-01 00:00:00）
    std::string start = req.has_param("start") ? req.get_param_value("start") : "";
    std::string end = req.has_param("end") ? req.get_param_value("end") : "";
    if (start.empty() || end.empty())
    {
        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errors;
        std::istringstream ss(req.body);
        Json::parseFromStream(reader, ss, &root, &errors);
        if (start.empty())
            start = root.get("start", "").asString();
        if (end.empty())
            end = root.get("end", "").asString();
    }
    if (start.empty() || end.empty())
    {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 400;
        e["message"] = "start/end 必填";
        Json::StreamWriterBuilder w;
        res.status = 400;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }
    auto fut = getThreadPool().enqueue([start, end]()
                                       { return getNewUserCountInRange(start, end); });
    int count = fut.get();
    if (count < 0)
    {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 500;
        e["message"] = "查询失败";
        Json::StreamWriterBuilder w;
        res.status = 500;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }
    Json::Value resp;
    resp["status"] = "success";
    resp["code"] = 200;
    resp["message"] = "OK";
    resp["data"]["newUsers"] = count;
    resp["data"]["start"] = start;
    resp["data"]["end"] = end;
    Json::StreamWriterBuilder w;
    res.status = 200;
    res.set_content(Json::writeString(w, resp), "application/json");
}

static bool requireAdmin(const httplib::Request &req, Json::Value &outUser)
{
    std::string token = req.has_header("Session-Token") ? req.get_header_value("Session-Token") : "";
    if (token.empty())
        return false;
    outUser = getUserInfoBySessionToken(token);
    std::string role = outUser.get("role", "").asString();
    return !outUser.isNull() && !outUser.get("userId", "").asString().empty() && role == "admin";
}

void handleAdminOverview(const httplib::Request &req, httplib::Response &res)
{
    Json::Value user;
    if (!requireAdmin(req, user))
    {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 403;
        e["message"] = "管理员权限不足";
        Json::StreamWriterBuilder w;
        res.status = 403;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }
    auto fut = getThreadPool().enqueue([]()
                                       { return getAdminOverviewStats(); });
    Json::Value stats = fut.get();
    if (!stats.get("ok", false).asBool())
    {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 500;
        e["message"] = "统计查询失败";
        Json::StreamWriterBuilder w;
        res.status = 500;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }
    Json::Value resp;
    resp["status"] = "success";
    resp["code"] = 200;
    resp["message"] = "OK";
    resp["data"] = stats;
    Json::StreamWriterBuilder w;
    res.status = 200;
    res.set_content(Json::writeString(w, resp), "application/json");
}

void handleAdminQueryUsers(const httplib::Request &req, httplib::Response &res)
{
    Json::Value user;
    if (!requireAdmin(req, user))
    {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 403;
        e["message"] = "管理员权限不足";
        Json::StreamWriterBuilder w;
        res.status = 403;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }
    std::string username = req.has_param("username") ? req.get_param_value("username") : "";
    std::string email = req.has_param("email") ? req.get_param_value("email") : "";
    std::string role = req.has_param("role") ? req.get_param_value("role") : "";
    int pageNum = req.has_param("PageNum") ? std::max(1, atoi(req.get_param_value("PageNum").c_str())) : 1;
    int pageSize = req.has_param("PageSize") ? std::min(100, std::max(1, atoi(req.get_param_value("PageSize").c_str()))) : 10;
    auto fut = getThreadPool().enqueue([=]()
                                       { return adminQueryUsers(username, email, role, pageNum, pageSize); });
    auto pair = fut.get();
    int total = pair.first;
    Json::Value list = pair.second;
    int totalPage = (total + pageSize - 1) / pageSize;
    if (totalPage <= 0)
        totalPage = 0;
    Json::Value resp;
    resp["status"] = "success";
    resp["code"] = 200;
    resp["message"] = "OK";
    resp["data"]["pageInfo"]["pageNum"] = pageNum;
    resp["data"]["pageInfo"]["pageSize"] = pageSize;
    resp["data"]["pageInfo"]["totalCount"] = total;
    resp["data"]["pageInfo"]["totalPage"] = totalPage;
    resp["data"]["list"] = list;
    Json::StreamWriterBuilder w;
    res.status = 200;
    res.set_content(Json::writeString(w, resp), "application/json");
}

void handleAdminQueryModels(const httplib::Request &req, httplib::Response &res)
{
    Json::Value user;
    if (!requireAdmin(req, user))
    {
        Json::Value e;
        e["status"] = "error";
        e["code"] = 403;
        e["message"] = "管理员权限不足";
        Json::StreamWriterBuilder w;
        res.status = 403;
        res.set_content(Json::writeString(w, e), "application/json");
        return;
    }
    std::string minLike = req.has_param("minLike") ? req.get_param_value("minLike") : "";
    std::string maxLike = req.has_param("maxLike") ? req.get_param_value("maxLike") : "";
    std::string minDownload = req.has_param("minDownload") ? req.get_param_value("minDownload") : "";
    std::string maxDownload = req.has_param("maxDownload") ? req.get_param_value("maxDownload") : "";
    std::string isPrivate = req.has_param("isPrivate") ? req.get_param_value("isPrivate") : "";
    int pageNum = req.has_param("PageNum") ? std::max(1, atoi(req.get_param_value("PageNum").c_str())) : 1;
    int pageSize = req.has_param("PageSize") ? std::min(100, std::max(1, atoi(req.get_param_value("PageSize").c_str()))) : 10;
    auto fut = getThreadPool().enqueue([=]()
                                       { return adminQueryModels(minLike, maxLike, minDownload, maxDownload, isPrivate, pageNum, pageSize); });
    auto pair = fut.get();
    int total = pair.first;
    Json::Value list = pair.second;
    int totalPage = (total + pageSize - 1) / pageSize;
    if (totalPage <= 0)
        totalPage = 0;
    Json::Value resp;
    resp["status"] = "success";
    resp["code"] = 200;
    resp["message"] = "OK";
    resp["data"]["pageInfo"]["pageNum"] = pageNum;
    resp["data"]["pageInfo"]["pageSize"] = pageSize;
    resp["data"]["pageInfo"]["totalCount"] = total;
    resp["data"]["pageInfo"]["totalPage"] = totalPage;
    resp["data"]["list"] = list;
    Json::StreamWriterBuilder w;
    res.status = 200;
    res.set_content(Json::writeString(w, resp), "application/json");
}