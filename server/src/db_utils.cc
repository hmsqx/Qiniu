#include "db_utils.h"
#include "config.h"

#include <mysql/mysql.h>
#include <iostream>
#include <stdexcept>

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
        snprintf(countSql, sizeof(countSql), "SELECT COUNT(*) AS total FROM ai3d_tasks WHERE user_id = '%s' AND version = '%s'", escapedUserId, escapedVersion);
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
                 "SELECT tx_job_id version FROM ai3d_tasks WHERE user_id = '%s' AND version = '%s' ORDER BY create_time DESC LIMIT %d, %d",
                 escapedUserId, escapedVersion, startIdx, pageSize);

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