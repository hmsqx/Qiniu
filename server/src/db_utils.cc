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
    MYSQL *conn = nullptr;
    MYSQL_RES *res = nullptr;
    MYSQL_ROW row = nullptr;

    conn = mysql_init(nullptr);
    if (conn == nullptr)
    {
        std::cerr << "MySQL 初始化失败：" << mysql_error(conn) << std::endl;
        return {totalCount, taskIdList};
    }

    const char *dbHost = MYSQL_HOST;
    const char *dbUser = MYSQL_USER;
    const char *dbPwd = MYSQL_PASSWORD;
    const char *dbName = MYSQL_DATABASE;
    unsigned int dbPort = MYSQL_PORT;

    if (mysql_real_connect(conn, dbHost, dbUser, dbPwd, dbName, dbPort, nullptr, 0) == nullptr)
    {
        std::cerr << "MySQL 连接失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return {totalCount, taskIdList};
    }

    if (mysql_set_character_set(conn, "utf8mb4") != 0)
    {
        std::cerr << "设置字符集失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return {totalCount, taskIdList};
    }

    try
    {
        char countSql[512];
        char escapedUserId[256];
        char escapedVersion[64];
        mysql_real_escape_string(conn, escapedUserId, userId.c_str(), userId.size());
        mysql_real_escape_string(conn, escapedVersion, version.c_str(), version.size());
        snprintf(countSql, sizeof(countSql), "SELECT COUNT(*) AS total FROM ai3d_tasks WHERE user_id = '%s'", escapedUserId);
        if (mysql_query(conn, countSql) != 0)
        {
            throw std::runtime_error(std::string("查询总条数失败：") + mysql_error(conn));
        }
        res = mysql_store_result(conn);
        if (res != nullptr && (row = mysql_fetch_row(res)) != nullptr)
        {
            totalCount = atoi(row[0]);
        }
        mysql_free_result(res);
        res = nullptr;

        int startIdx = (pageNum - 1) * pageSize;
        char pageSql[768];
        snprintf(pageSql, sizeof(pageSql),
                 "SELECT tx_job_id ,version FROM ai3d_tasks WHERE user_id = '%s' ORDER BY create_time DESC LIMIT %d, %d",
                 escapedUserId,  startIdx, pageSize);

        if (mysql_query(conn, pageSql) != 0)
        {
            throw std::runtime_error(std::string("分页查询任务ID失败：") + mysql_error(conn));
        }
        res = mysql_store_result(conn);
        if (res == nullptr)
        {
            throw std::runtime_error(std::string("获取分页结果集失败：") + mysql_error(conn));
        }
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            taskIdList.push_back({row[0],row[1]});
        }
        mysql_free_result(res);
    }
    catch (const std::exception &e)
    {
        std::cerr << "MySQL 查询异常：" << e.what() << std::endl;
        totalCount = -1;
    }

    mysql_close(conn);
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
    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr)
    {
        std::cerr << "MySQL 初始化失败：" << mysql_error(conn) << std::endl;
        return false;
    }

    if (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, MYSQL_PORT, nullptr, 0) == nullptr)
    {
        std::cerr << "MySQL 连接失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return false;
    }

    if (mysql_set_character_set(conn, "utf8mb4") != 0)
    {
        std::cerr << "设置字符集失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return false;
    }

    // 转义
    auto escape = [&](const std::string &s) {
        std::string out;
        out.resize(s.size() * 2 + 1);
        unsigned long len = mysql_real_escape_string(conn, &out[0], s.c_str(), s.size());
        out.resize(len);
        return out;
    };

    std::string eUser = escape(userId);
    std::string eJob = escape(jobId);
    std::string eReq = escape(requestId);
    std::string ePrompt = escape(prompt);
    std::string eFmt = escape(resultFormat);
    std::string eStatus = escape(status);
    std::string eVer = escape(version);

    std::string sql = "INSERT INTO ai3d_tasks (user_id, tx_job_id, request_id, status, prompt, result_format, version, create_time, update_time, deleted) VALUES (" \
                      "'" + eUser + "', '" + eJob + "', '" + eReq + "', '" + eStatus + "', '" + ePrompt + "', '" + eFmt + "', '" + eVer + "', NOW(), NOW(), 0)";

    bool ok = mysql_query(conn, sql.c_str()) == 0;
    if (!ok)
    {
        std::cerr << "插入任务失败：" << mysql_error(conn) << std::endl;
    }
    mysql_close(conn);
    return ok;
}

static bool execUpdateSql(const std::string &sql)
{
    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr)
    {
        std::cerr << "MySQL 初始化失败：" << mysql_error(conn) << std::endl;
        return false;
    }
    if (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, MYSQL_PORT, nullptr, 0) == nullptr)
    {
        std::cerr << "MySQL 连接失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return false;
    }
    if (mysql_set_character_set(conn, "utf8mb4") != 0)
    {
        std::cerr << "设置字符集失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return false;
    }
    bool ok = mysql_query(conn, sql.c_str()) == 0;
    if (!ok)
    {
        std::cerr << "执行更新失败：" << mysql_error(conn) << std::endl;
    }
    mysql_close(conn);
    return ok;
}

bool updateAi3dTaskStatus(const std::string &jobId, const std::string &status)
{
    // 简单转义
    std::string eJobId, eStatus;
    eJobId.resize(jobId.size() * 2 + 1);
    eStatus.resize(status.size() * 2 + 1);

    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr) return false;
    if (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, MYSQL_PORT, nullptr, 0) == nullptr) { mysql_close(conn); return false; }
    unsigned long jlen = mysql_real_escape_string(conn, &eJobId[0], jobId.c_str(), jobId.size());
    unsigned long slen = mysql_real_escape_string(conn, &eStatus[0], status.c_str(), status.size());
    eJobId.resize(jlen);
    eStatus.resize(slen);
    mysql_close(conn);
    std::string sql = "UPDATE ai3d_tasks SET status='" + eStatus + "', update_time=NOW() WHERE tx_job_id='" + eJobId + "'";
    return execUpdateSql(sql);
}

bool updateAi3dTaskError(const std::string &jobId, const std::string &errorMessage)
{
    std::string eJobId, eErr;
    eJobId.resize(jobId.size() * 2 + 1);
    eErr.resize(errorMessage.size() * 2 + 1);

    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr) return false;
    if (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, MYSQL_PORT, nullptr, 0) == nullptr) { mysql_close(conn); return false; }
    unsigned long jlen = mysql_real_escape_string(conn, &eJobId[0], jobId.c_str(), jobId.size());
    unsigned long elen = mysql_real_escape_string(conn, &eErr[0], errorMessage.c_str(), errorMessage.size());
    eJobId.resize(jlen);
    eErr.resize(elen);
    mysql_close(conn);
    std::string sql = "UPDATE ai3d_tasks SET status='FAILED', error_message='" + eErr + "', update_time=NOW() WHERE tx_job_id='" + eJobId + "'";
    return execUpdateSql(sql);
} 

bool tryConsumeUserTokenOnce(const std::string &userId)
{
    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr)
    {
        std::cerr << "MySQL 初始化失败：" << mysql_error(conn) << std::endl;
        return false;
    }
    if (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, MYSQL_PORT, nullptr, 0) == nullptr)
    {
        std::cerr << "MySQL 连接失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return false;
    }
    if (mysql_set_character_set(conn, "utf8mb4") != 0)
    {
        std::cerr << "设置字符集失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return false;
    }

    // 简单转义 userId
    std::string eUserId;
    eUserId.resize(userId.size() * 2 + 1);
    unsigned long ulen = mysql_real_escape_string(conn, &eUserId[0], userId.c_str(), userId.size());
    eUserId.resize(ulen);

    // 原子扣减：仅当 token_count > 0 时更新
    std::ostringstream oss;
    oss << "UPDATE users SET token_count = token_count - 1, update_time=NOW() "
        << "WHERE user_id='" << eUserId << "' AND token_count > 0";
    std::string sql = oss.str();
    if (mysql_query(conn, sql.c_str()) != 0)
    {
        std::cerr << "扣减 token 失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return false;
    }
    my_ulonglong affected = mysql_affected_rows(conn);
    mysql_close(conn);
    return affected > 0;
}

Json::Value getUserInfoBySessionToken(const std::string &sessionToken)
{
    Json::Value info;
    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr)
    {
        std::cerr << "MySQL 初始化失败：" << mysql_error(conn) << std::endl;
        return info;
    }
    if (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, MYSQL_PORT, nullptr, 0) == nullptr)
    {
        std::cerr << "MySQL 连接失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return info;
    }
    if (mysql_set_character_set(conn, "utf8mb4") != 0)
    {
        std::cerr << "设置字符集失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return info;
    }

    // 转义 token
    std::string eToken;
    eToken.resize(sessionToken.size() * 2 + 1);
    unsigned long tlen = mysql_real_escape_string(conn, &eToken[0], sessionToken.c_str(), sessionToken.size());
    eToken.resize(tlen);

    // 关联查询 users 与 user_sessions（未撤销且未过期）
    std::ostringstream oss;
    oss << "SELECT u.user_id, u.username, u.role, u.token_count "
        << "FROM users u JOIN user_sessions s ON u.user_id = s.user_id "
        << "WHERE s.session_token='" << eToken << "' AND s.revoked=0 AND s.expire_time > NOW() LIMIT 1";
    std::string sql = oss.str();
    if (mysql_query(conn, sql.c_str()) != 0)
    {
        std::cerr << "查询用户信息失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return info;
    }
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row = res ? mysql_fetch_row(res) : nullptr;
    if (row)
    {
        info["userId"] = row[0] ? row[0] : "";
        info["username"] = row[1] ? row[1] : "";
        info["role"] = row[2] ? row[2] : "";
        info["token_count"] = row[3] ? atoi(row[3]) : 0;
    }
    if (res) mysql_free_result(res);
    mysql_close(conn);
    return info;
}

bool incrementModelDownloadCount(const std::string &jobId)
{
    std::string eJobId;
    eJobId.resize(jobId.size() * 2 + 1);
    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr) return false;
    if (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, MYSQL_PORT, nullptr, 0) == nullptr) { mysql_close(conn); return false; }
    unsigned long jlen = mysql_real_escape_string(conn, &eJobId[0], jobId.c_str(), jobId.size());
    eJobId.resize(jlen);
    mysql_close(conn);
    std::string sql = "UPDATE ai3d_tasks SET downloadCount = COALESCE(downloadCount,0) + 1, update_time=NOW() WHERE tx_job_id='" + eJobId + "'";
    return execUpdateSql(sql);
}

bool incrementModelLikeCount(const std::string &jobId)
{
    std::string eJobId;
    eJobId.resize(jobId.size() * 2 + 1);
    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr) return false;
    if (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, MYSQL_PORT, nullptr, 0) == nullptr) { mysql_close(conn); return false; }
    unsigned long jlen = mysql_real_escape_string(conn, &eJobId[0], jobId.c_str(), jobId.size());
    eJobId.resize(jlen);
    mysql_close(conn);
    std::string sql = "UPDATE ai3d_tasks SET `like` = COALESCE(`like`,0) + 1, update_time=NOW() WHERE tx_job_id='" + eJobId + "'";
    return execUpdateSql(sql);
}

std::pair<int, Json::Value> queryModelsByPrivacy(bool isPrivate,
                                                 int pageNum,
                                                 int pageSize)
{
    Json::Value list(Json::arrayValue);
    int total = 0;
    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr)
    {
        std::cerr << "MySQL 初始化失败：" << mysql_error(conn) << std::endl;
        return {0, list};
    }
    if (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, MYSQL_PORT, nullptr, 0) == nullptr)
    {
        std::cerr << "MySQL 连接失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return {0, list};
    }
    if (mysql_set_character_set(conn, "utf8mb4") != 0)
    {
        std::cerr << "设置字符集失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return {0, list};
    }

    // 统计总数
    std::ostringstream countSql;
    countSql << "SELECT COUNT(*) FROM ai3d_tasks WHERE COALESCE(Isprivate, 0) = " << (isPrivate ? 1 : 0);
    if (mysql_query(conn, countSql.str().c_str()) != 0)
    {
        std::cerr << "统计模型数量失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return {0, list};
    }
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row = res ? mysql_fetch_row(res) : nullptr;
    if (row) total = atoi(row[0]);
    if (res) mysql_free_result(res);

    int startIdx = (pageNum - 1) * pageSize;
    std::ostringstream pageSql;
    pageSql << "SELECT tx_job_id, user_id, status, result_format, version, COALESCE(downloadCount,0), COALESCE(Isprivate,0), COALESCE(`like`,0), create_time "
            << "FROM ai3d_tasks WHERE COALESCE(Isprivate, 0) = " << (isPrivate ? 1 : 0)
            << " ORDER BY create_time DESC LIMIT " << startIdx << ", " << pageSize;
    if (mysql_query(conn, pageSql.str().c_str()) != 0)
    {
        std::cerr << "分页查询模型失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return {0, list};
    }
    res = mysql_store_result(conn);
    while ((row = mysql_fetch_row(res)) != nullptr)
    {
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
    if (res) mysql_free_result(res);
    mysql_close(conn);
    return {total, list};
}

bool toggleJobIsPrivate(const std::string& jobId)
{
    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr)
    {
        std::cerr << "MySQL 初始化失败：" << mysql_error(conn) << std::endl;
        return false;
    }
    if (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, MYSQL_PORT, nullptr, 0) == nullptr)
    {
        std::cerr << "MySQL 连接失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return false;
    }
    if (mysql_set_character_set(conn, "utf8mb4") != 0)
    {
        std::cerr << "设置字符集失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return false;
    }
    std::string escJobId;
    escJobId.resize(jobId.size() * 2 + 1);
    unsigned long jlen = mysql_real_escape_string(conn, &escJobId[0], jobId.c_str(), jobId.size());
    escJobId.resize(jlen);
    std::string query = "UPDATE ai3d_tasks SET Isprivate = NOT Isprivate WHERE tx_job_id='" + escJobId + "'";
    bool ok = (mysql_query(conn, query.c_str()) == 0);
    mysql_close(conn);
    return ok;
}