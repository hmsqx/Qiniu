#include "handlers.h"
#include "connection_pool.h"
#include "transaction_manager.h"
#include "thread_pool.h"
#include "api_security.h"
#include <future>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <sstream>
#include "db_utils.h"
#include "tx_ai3d.h"

// 优化的数据库查询函数，使用连接池
Json::Value getTaskCompleteInfoOptimized(const std::string& jobId)
{
    Json::Value result;

    try {
        // 使用连接池获取连接
        ScopedConnection conn;
        if (!conn.isValid()) {
            std::cerr << "数据库连接失败" << std::endl;
            result["found"] = false;
            return result;
        }

        // 转义输入以防止SQL注入
        std::string eJobId = conn.escapeString(jobId);

        std::ostringstream sql;
        sql << "SELECT fileurl, previewImages, Isprivate, status, prompt, result_format, version, create_time, "
            << "COALESCE(downloadCount,0), COALESCE(`like`,0) "
            << "FROM ai3d_tasks WHERE tx_job_id='" << eJobId << "' LIMIT 1";

        auto res = conn.executeQuery(sql.str());
        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res.get());
            if (row) {
                result["jobId"] = jobId;
                result["fileurl"] = row[0] ? row[0] : "";
                result["previewImages"] = row[1] ? row[1] : "";
                result["Isprivate"] = (row[2] && atoi(row[2]) != 0);
                result["status"] = row[3] ? row[3] : "";
                result["prompt"] = row[4] ? row[4] : "";
                result["resultFormat"] = row[5] ? row[5] : "";
                result["version"] = row[6] ? row[6] : "";
                result["createTime"] = row[7] ? row[7] : "";
                result["downloadCount"] = row[8] ? atoi(row[8]) : 0;
                result["like"] = row[9] ? atoi(row[9]) : 0;
                result["found"] = true;
            } else {
                result["found"] = false;
            }
        } else {
            result["found"] = false;
        }
    } catch (const std::exception& e) {
        std::cerr << "查询任务信息异常: " << e.what() << std::endl;
        result["found"] = false;
    }

    return result;
}

// 使用事务的原子操作
bool updateTaskFilesWithTransaction(const std::string& jobId,
                                   const std::string& fileUrl,
                                   const std::string& previewImages) {
    return executeWithRetry([&]() -> bool {
        ScopedConnection conn;
        if (!conn.isValid()) {
            return false;
        }

        TransactionManager transaction(std::move(conn));
        if (!transaction.begin(IsolationLevel::REPEATABLE_READ)) {
            return false;
        }

        try {
            std::string eJobId = transaction.escapeString(jobId);
            std::string eFileUrl = transaction.escapeString(fileUrl);
            std::string ePreviewImages = transaction.escapeString(previewImages);

            std::string sql = "UPDATE ai3d_tasks SET fileurl='" + eFileUrl +
                            "', previewImages='" + ePreviewImages +
                            "', update_time=NOW() WHERE tx_job_id='" + eJobId + "'";

            if (!transaction.executeUpdate(sql)) {
                transaction.rollback();
                return false;
            }

            return transaction.commit();
        } catch (const std::exception& e) {
            std::cerr << "更新任务文件异常: " << e.what() << std::endl;
            transaction.rollback();
            return false;
        }
    });
}

// 异步处理查询请求
// void handleQueryJobsByPageAsync(const httplib::Request &req, httplib::Response &res) {
//     // 参数验证
//     std::string userId = req.get_param_value("UserId");
//     std::string pageNumStr = req.get_param_value("PageNum");
//     std::string pageSizeStr = req.get_param_value("PageSize");

//     if (userId.empty() || pageNumStr.empty() || pageSizeStr.empty()) {
//         Json::Value errorResponse;
//         errorResponse["status"] = "error";
//         errorResponse["code"] = 400;
//         errorResponse["message"] = "参数不完整";
//         Json::StreamWriterBuilder writer;
//         res.status = 400;
//         res.set_content(Json::writeString(writer, errorResponse), "application/json");
//         return;
//     }

//     int pageNum, pageSize;
//     try {
//         pageNum = std::stoi(pageNumStr);
//         pageSize = std::stoi(pageSizeStr);
//     } catch (const std::exception& e) {
//         Json::Value errorResponse;
//         errorResponse["status"] = "error";
//         errorResponse["code"] = 400;
//         errorResponse["message"] = "参数格式错误";
//         Json::StreamWriterBuilder writer;
//         res.status = 400;
//         res.set_content(Json::writeString(writer, errorResponse), "application/json");
//         return;
//     }

//     // 使用线程池异步处理
//     auto future = getThreadPool().enqueue([=]() -> Json::Value {
//         try {
//             // 获取任务列表
//             auto taskIds = getTaskIdsByMySQLCAPI(userId, pageNum, pageSize, "");
//             if (taskIds.first == 0) {
//                 Json::Value result;
//                 result["status"] = "success";
//                 result["code"] = 200;
//                 result["message"] = "查询成功";
//                 result["data"]["total"] = 0;
//                 result["data"]["pageNum"] = pageNum;
//                 result["data"]["pageSize"] = pageSize;
//                 result["data"]["tasks"] = Json::Value(Json::arrayValue);
//                 return result;
//             }

//             Json::Value currentPageData;
//             for (const auto &job : taskIds.second) {
//                 // 使用优化的查询函数
//                 Json::Value dbInfo = getTaskCompleteInfoOptimized(job.first);

//                 // 获取腾讯云状态
//                 Json::Value taskInfo;
//                 if (job.second == "rapid") {
//                     taskInfo = queryTaskStatusFromTxRapid(job.first);
//                 } else if (job.second == "pro") {
//                     taskInfo = queryTaskStatusFromTxPro(job.first);
//                 } else {
//                     taskInfo = queryTaskStatusFromTx(job.first);
//                 }

//                 // 合并信息
//                 if (dbInfo.get("found", false).asBool()) {
//                     taskInfo["fileurl"] = dbInfo["fileurl"];
//                     if (!dbInfo["previewImages"].empty()) {
//                         taskInfo["previewImages"] = dbInfo["previewImages"];
//                     }
//                     taskInfo["Isprivate"] = dbInfo["Isprivate"];
//                     taskInfo["downloadCount"] = dbInfo["downloadCount"];
//                     taskInfo["like"] = dbInfo["like"];
//                     taskInfo["createTime"] = dbInfo["createTime"];
//                 }

//                 currentPageData.append(taskInfo);
//             }

//             Json::Value result;
//             result["status"] = "success";
//             result["code"] = 200;
//             result["message"] = "查询成功";
//             result["data"]["total"] = taskIds.first;
//             result["data"]["pageNum"] = pageNum;
//             result["data"]["pageSize"] = pageSize;
//             result["data"]["tasks"] = currentPageData;

//             return result;
//         } catch (const std::exception& e) {
//             Json::Value errorResult;
//             errorResult["status"] = "error";
//             errorResult["code"] = 500;
//             errorResult["message"] = std::string("服务器内部错误: ") + e.what();
//             return errorResult;
//         }
//     });

//     // 等待结果
//     try {
//         Json::Value result = future.get();
//         Json::StreamWriterBuilder writer;
//         res.set_content(Json::writeString(writer, result), "application/json");

//         if (result["status"].asString() == "error") {
//             res.status = result["code"].asInt();
//         } else {
//             res.status = 200;
//         }
//     } catch (const std::exception& e) {
//         Json::Value errorResponse;
//         errorResponse["status"] = "error";
//         errorResponse["code"] = 500;
//         errorResponse["message"] = "异步处理失败";
//         Json::StreamWriterBuilder writer;
//         res.status = 500;
//         res.set_content(Json::writeString(writer, errorResponse), "application/json");
//     }
// }



// 并发安全的模型下载计数
bool incrementDownloadCountSafely(const std::string& jobId) {
    return executeWithRetry([&]() -> bool {
        ScopedConnection conn;
        if (!conn.isValid()) {
            return false;
        }

        TransactionManager transaction(std::move(conn));
        if (!transaction.begin(IsolationLevel::REPEATABLE_READ)) {
            return false;
        }

        try {
            std::string eJobId = transaction.escapeString(jobId);

            // 使用原子操作更新计数
            std::string sql = "UPDATE ai3d_tasks SET downloadCount=COALESCE(downloadCount,0)+1 WHERE tx_job_id='" + eJobId + "'";

            if (!transaction.executeUpdate(sql)) {
                transaction.rollback();
                return false;
            }

            return transaction.commit();
        } catch (const std::exception& e) {
            std::cerr << "增加下载计数异常: " << e.what() << std::endl;
            transaction.rollback();
            return false;
        }
    });
}
