#include "db_utils.h"
#include "config.h"

#include <mysql/mysql.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <jsoncpp/json/json.h>

std::pair<int, std::vector<std::pair<std::string, std::string>>> getTaskIdsByMySQLCAPI(const std::string &userId,
                                                              int pageNum,
                                                              int pageSize,
                                                              const std::string &version)
{
    std::vector<std::pair<std::string, std::string>> taskIdList;
    int totalCount = -1;
    
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        std::cerr << "数据库连接失败" << std::endl;
        return {totalCount, taskIdList};
    }

    try {
        // 查询总数
        std::string escapedUserId = conn->escapeString(userId);
        std::string countSql = "SELECT COUNT(*) AS total FROM ai3d_tasks WHERE user_id = '" + escapedUserId + "'";
        
        auto res = conn->executeQuery(countSql);
        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res.get());
            if (row) {
                totalCount = atoi(row[0]);
            }
        }

        // 分页查询
        int startIdx = (pageNum - 1) * pageSize;
        std::ostringstream pageSql;
        pageSql << "SELECT tx_job_id, version FROM ai3d_tasks WHERE user_id = '" << escapedUserId 
                << "' ORDER BY create_time DESC LIMIT " << startIdx << ", " << pageSize;

        res = conn->executeQuery(pageSql.str());
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res.get())) != nullptr) {
                taskIdList.push_back({row[0] ? row[0] : "", row[1] ? row[1] : ""});
            }
        }
    }
    catch (const std::exception &e) {
        std::cerr << "MySQL 查询异常：" << e.what() << std::endl;
        totalCount = -1;
    }

    return {totalCount, taskIdList};
}

bool insertAi3dTask(const std::string &userId,
                    const std::string &jobId,
                    const std::string &requestId,
                    const std::string &prompt,
                    const std::string &resultFormat,
                    const std::string &status,
                    const std::string &version)
{
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        std::cerr << "数据库连接失败" << std::endl;
        return false;
    }

    // 转义所有参数
    std::string eUser = conn->escapeString(userId);
    std::string eJob = conn->escapeString(jobId);
    std::string eReq = conn->escapeString(requestId);
    std::string ePrompt = conn->escapeString(prompt);
    std::string eFmt = conn->escapeString(resultFormat);
    std::string eStatus = conn->escapeString(status);
    std::string eVer = conn->escapeString(version);

    std::ostringstream sql;
    sql << "INSERT INTO ai3d_tasks (user_id, tx_job_id, request_id, status, prompt, result_format, version, create_time, update_time, deleted) VALUES ("
        << "'" << eUser << "', '" << eJob << "', '" << eReq << "', '" << eStatus << "', '" 
        << ePrompt << "', '" << eFmt << "', '" << eVer << "', NOW(), NOW(), 0)";

    bool ok = conn->executeUpdate(sql.str());
    if (!ok) {
        std::cerr << "插入任务失败" << std::endl;
    }
    return ok;
}

bool updateAi3dTaskStatus(const std::string &jobId, const std::string &status)
{
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        return false;
    }
    
    std::string eJobId = conn->escapeString(jobId);
    std::string eStatus = conn->escapeString(status);
    
    std::ostringstream sql;
    sql << "UPDATE ai3d_tasks SET status='" << eStatus << "', update_time=NOW() WHERE tx_job_id='" << eJobId << "'";
    
    return conn->executeUpdate(sql.str());
}

bool updateAi3dTaskError(const std::string &jobId, const std::string &errorMessage)
{
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        return false;
    }
    
    std::string eJobId = conn->escapeString(jobId);
    std::string eErr = conn->escapeString(errorMessage);
    
    std::ostringstream sql;
    sql << "UPDATE ai3d_tasks SET status='FAILED', error_message='" << eErr << "', update_time=NOW() WHERE tx_job_id='" << eJobId << "'";
    
    return conn->executeUpdate(sql.str());
}

bool updateAi3dTaskFiles(const std::string &jobId, 
                        const std::string &fileUrl, 
                        const std::string &previewImages)
{
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        return false;
    }
    
    std::string eJobId = conn->escapeString(jobId);
    std::string eFileUrl = conn->escapeString(fileUrl);
    std::string ePreviewImages = conn->escapeString(previewImages);
    
    std::ostringstream sql;
    sql << "UPDATE ai3d_tasks SET fileurl='" << eFileUrl << "', previewImages='" << ePreviewImages 
        << "', update_time=NOW() WHERE tx_job_id='" << eJobId << "'";
    
    return conn->executeUpdate(sql.str());
}

bool tryConsumeUserTokenOnce(const std::string &userId)
{
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        std::cerr << "数据库连接失败" << std::endl;
        return false;
    }

    std::string eUserId = conn->escapeString(userId);
    
    // 原子扣减：仅当 token_count > 0 时更新
    std::ostringstream sql;
    sql << "UPDATE users SET token_count = token_count - 1, update_time=NOW() "
        << "WHERE user_id='" << eUserId << "' AND token_count > 0";
    
    bool ok = conn->executeUpdate(sql.str());
    if (ok) {
        my_ulonglong affected = conn->getAffectedRows();
        return affected > 0;
    }
    return false;
}

Json::Value getUserInfoBySessionToken(const std::string &sessionToken)
{
    Json::Value info;
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        std::cerr << "数据库连接失败" << std::endl;
        return info;
    }

    std::string eToken = conn->escapeString(sessionToken);

    // 关联查询 users 与 user_sessions（未撤销且未过期）
    std::ostringstream sql;
    sql << "SELECT u.user_id, u.username, u.role, u.token_count "
        << "FROM users u JOIN user_sessions s ON u.user_id = s.user_id "
        << "WHERE s.session_token='" << eToken << "' AND s.revoked=0 AND s.expire_time > NOW() LIMIT 1";
    
    auto res = conn->executeQuery(sql.str());
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res.get());
        if (row) {
            info["userId"] = row[0] ? row[0] : "";
            info["username"] = row[1] ? row[1] : "";
            info["role"] = row[2] ? row[2] : "";
            info["token_count"] = row[3] ? atoi(row[3]) : 0;
        }
    }
    
    return info;
}

bool incrementModelDownloadCount(const std::string &jobId)
{
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        return false;
    }
    
    std::string eJobId = conn->escapeString(jobId);
    
    std::ostringstream sql;
    sql << "UPDATE ai3d_tasks SET downloadCount = COALESCE(downloadCount,0) + 1, update_time=NOW() WHERE tx_job_id='" << eJobId << "'";
    
    return conn->executeUpdate(sql.str());
}

bool incrementModelLikeCount(const std::string &jobId)
{
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        return false;
    }
    
    std::string eJobId = conn->escapeString(jobId);
    
    std::ostringstream sql;
    sql << "UPDATE ai3d_tasks SET `like` = COALESCE(`like`,0) + 1, update_time=NOW() WHERE tx_job_id='" << eJobId << "'";
    
    return conn->executeUpdate(sql.str());
}

std::pair<int, Json::Value> queryModelsByPrivacy(bool isPrivate,
                                                 int pageNum,
                                                 int pageSize)
{
    Json::Value list(Json::arrayValue);
    int total = 0;
    
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        std::cerr << "数据库连接失败" << std::endl;
        return {0, list};
    }

    // 统计总数
    std::ostringstream countSql;
    countSql << "SELECT COUNT(*) FROM ai3d_tasks WHERE COALESCE(Isprivate, 0) = " << (isPrivate ? 1 : 0);
    
    auto res = conn->executeQuery(countSql.str());
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res.get());
        if (row) total = atoi(row[0]);
    }

    // 分页查询
    int startIdx = (pageNum - 1) * pageSize;
    std::ostringstream pageSql;
    pageSql << "SELECT tx_job_id, user_id, status, result_format, version, COALESCE(downloadCount,0), COALESCE(Isprivate,0), COALESCE(`like`,0), create_time "
            << "FROM ai3d_tasks WHERE COALESCE(Isprivate, 0) = " << (isPrivate ? 1 : 0)
            << " ORDER BY create_time DESC LIMIT " << startIdx << ", " << pageSize;
    
    res = conn->executeQuery(pageSql.str());
    if (res) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res.get())) != nullptr) {
            Json::Value item;
            item["jobId"] = row[0] ? row[0] : "";
            item["userId"] = row[1] ? row[1] : "";
            item["status"] = row[2] ? row[2] : "";
            item["resultFormat"] = row[3] ? row[3] : "";
            item["version"] = row[4] ? row[4] : "";
            item["downloadCount"] = row[5] ? atoi(row[5]) : 0;
            item["Isprivate"] = (row[6] && atoi(row[6]) != 0);
            item["like"] = row[7] ? atoi(row[7]) : 0;
            item["create_time"] = row[8] ? row[8] : "";
            list.append(item);
        }
    }
    
    return {total, list};
}

bool toggleJobIsPrivate(const std::string& jobId)
{
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        std::cerr << "数据库连接失败" << std::endl;
        return false;
    }
    
    std::string eJobId = conn->escapeString(jobId);
    
    std::ostringstream sql;
    sql << "UPDATE ai3d_tasks SET Isprivate = NOT Isprivate WHERE tx_job_id='" << eJobId << "'";
    
    return conn->executeUpdate(sql.str());
}

Json::Value getTaskFileInfo(const std::string& jobId)
{
    Json::Value result;
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        std::cerr << "数据库连接失败" << std::endl;
        return result;
    }
    
    std::string eJobId = conn->escapeString(jobId);
    
    std::ostringstream sql;
    sql << "SELECT fileurl, previewImages, status, prompt, result_format, version, create_time, Isprivate "
        << "FROM ai3d_tasks WHERE tx_job_id='" << eJobId << "' LIMIT 1";
    
    auto res = conn->executeQuery(sql.str());
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res.get());
        if (row) {
            result["jobId"] = jobId;
            result["fileurl"] = row[0] ? row[0] : "";
            result["previewImages"] = row[1] ? row[1] : "";
            result["status"] = row[2] ? row[2] : "";
            result["prompt"] = row[3] ? row[3] : "";
            result["resultFormat"] = row[4] ? row[4] : "";
            result["version"] = row[5] ? row[5] : "";
            result["createTime"] = row[6] ? row[6] : "";
            result["Isprivate"] = (row[6] && atoi(row[6]) != 0);
            result["found"] = true;
        } else {
            result["found"] = false;
        }
    }
    
    return result;
}

Json::Value getTaskCompleteInfo(const std::string& jobId)
{
    Json::Value result;
    auto conn = createDatabaseConnection();
    if (!conn || !conn->isConnected()) {
        std::cerr << "数据库连接失败" << std::endl;
        return result;
    }
    
    std::string eJobId = conn->escapeString(jobId);
    
    std::ostringstream sql;
    sql << "SELECT fileurl, previewImages, Isprivate, status, prompt, result_format, version, create_time, "
        << "COALESCE(downloadCount,0), COALESCE(`like`,0) "
        << "FROM ai3d_tasks WHERE tx_job_id='" << eJobId << "' LIMIT 1";
    
    auto res = conn->executeQuery(sql.str());
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
    }
    
    return result;
}
