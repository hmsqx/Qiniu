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
    
    ScopedConnection conn;
    if (!conn.isValid()) {
        std::cerr << "数据库连接失败" << std::endl;
        return {totalCount, taskIdList};
    }

    try {
        // 查询总数
        std::string escapedUserId = conn.escapeString(userId);
        std::string countSql = "SELECT COUNT(*) AS total FROM ai3d_tasks WHERE user_id = '" + escapedUserId + "'";
        
        auto res = conn.executeQuery(countSql);
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

        res = conn.executeQuery(pageSql.str());
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
    ScopedConnection conn;
    if (!conn.isValid()) {
        std::cerr << "数据库连接失败" << std::endl;
        return false;
    }

    // 转义所有参数
    std::string eUser = conn.escapeString(userId);
    std::string eJob = conn.escapeString(jobId);
    std::string eReq = conn.escapeString(requestId);
    std::string ePrompt = conn.escapeString(prompt);
    std::string eFmt = conn.escapeString(resultFormat);
    std::string eStatus = conn.escapeString(status);
    std::string eVer = conn.escapeString(version);

    std::ostringstream sql;
    sql << "INSERT INTO ai3d_tasks (user_id, tx_job_id, request_id, status, prompt, result_format, version, create_time, update_time, deleted) VALUES ("
        << "'" << eUser << "', '" << eJob << "', '" << eReq << "', '" << eStatus << "', '" 
        << ePrompt << "', '" << eFmt << "', '" << eVer << "', NOW(), NOW(), 0)";

    bool ok = conn.executeUpdate(sql.str());
    if (!ok) {
        std::cerr << "插入任务失败" << std::endl;
    }
    return ok;
}

bool updateAi3dTaskStatus(const std::string &jobId, const std::string &status)
{
    ScopedConnection conn;
    if (!conn.isValid()) {
        return false;
    }
    
    std::string eJobId = conn.escapeString(jobId);
    std::string eStatus = conn.escapeString(status);
    
    std::ostringstream sql;
    sql << "UPDATE ai3d_tasks SET status='" << eStatus << "', update_time=NOW() WHERE tx_job_id='" << eJobId << "'";
    
    return conn.executeUpdate(sql.str());
}

bool updateAi3dTaskError(const std::string &jobId, const std::string &errorMessage)
{
    ScopedConnection conn;
    if (!conn.isValid()) {
        return false;
    }
    
    std::string eJobId = conn.escapeString(jobId);
    std::string eErr = conn.escapeString(errorMessage);
    
    std::ostringstream sql;
    sql << "UPDATE ai3d_tasks SET status='FAILED', error_message='" << eErr << "', update_time=NOW() WHERE tx_job_id='" << eJobId << "'";
    
    return conn.executeUpdate(sql.str());
}

bool updateAi3dTaskFiles(const std::string &jobId, 
                        const std::string &fileUrl, 
                        const std::string &previewImages)
{
    ScopedConnection conn;
    if (!conn.isValid()) {
        return false;
    }
    
    std::string eJobId = conn.escapeString(jobId);
    std::string eFileUrl = conn.escapeString(fileUrl);
    std::string ePreviewImages = conn.escapeString(previewImages);
    
    std::ostringstream sql;
    sql << "UPDATE ai3d_tasks SET fileurl='" << eFileUrl << "', previewImages='" << ePreviewImages 
        << "', update_time=NOW() WHERE tx_job_id='" << eJobId << "'";
    
    return conn.executeUpdate(sql.str());
}

// 并发安全的用户token消费
bool consumeUserTokenSafely(const std::string& userId) {
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
            std::string eUserId = transaction.escapeString(userId);
            
            // 检查当前token数量（加行级锁）
            std::string checkSql = "SELECT token_count FROM users WHERE user_id='" + eUserId + "' FOR UPDATE";
            auto checkRes = transaction.executeQuery(checkSql);
            if (!checkRes) {
                transaction.rollback();
                return false;
            }
            
            MYSQL_ROW row = mysql_fetch_row(checkRes.get());
            if (!row || !row[0]) {
                transaction.rollback();
                return false;
            }
            
            int beforeTokens = atoi(row[0]);
            if (beforeTokens <= 0) {
                transaction.rollback();
                return false; // 余额不足
            }
            
            // 扣减token（条件更新，确保并发安全）
            std::string updateSql = "UPDATE users SET token_count=token_count-1, update_time=NOW() WHERE user_id='" + eUserId + "' AND token_count > 0";
            if (!transaction.executeUpdate(updateSql)) {
                transaction.rollback();
                return false;
            }
            
            // 重新查询校验是否确实扣减了1（避免依赖 getAffectedRows）
            auto verifyRes = transaction.executeQuery(checkSql);
            if (!verifyRes) {
                transaction.rollback();
                return false;
            }
            MYSQL_ROW verifyRow = mysql_fetch_row(verifyRes.get());
            if (!verifyRow || !verifyRow[0]) {
                transaction.rollback();
                return false;
            }
            int afterTokens = atoi(verifyRow[0]);
            if (afterTokens != beforeTokens - 1) {
                transaction.rollback();
                return false; // 未按预期扣减
            }
            
            return transaction.commit();
        } catch (const std::exception& e) {
            std::cerr << "消费用户token异常: " << e.what() << std::endl;
            transaction.rollback();
            return false;
        }
    });
}



Json::Value getUserInfoBySessionToken(const std::string &sessionToken)
{
    Json::Value info;
    ScopedConnection conn;
    if (!conn.isValid()) {
        std::cerr << "数据库连接失败" << std::endl;
        return info;
    }

    std::string eToken = conn.escapeString(sessionToken);

    // 关联查询 users 与 user_sessions（未撤销且未过期）
    std::ostringstream sql;
    sql << "SELECT u.user_id, u.username, u.role, u.token_count "
        << "FROM users u JOIN user_sessions s ON u.user_id = s.user_id "
        << "WHERE s.session_token='" << eToken << "' AND s.revoked=0 AND s.expire_time > NOW() LIMIT 1";
    
    auto res = conn.executeQuery(sql.str());
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
    ScopedConnection conn;
    if (!conn.isValid()) {
        return false;
    }
    
    std::string eJobId = conn.escapeString(jobId);
    
    std::ostringstream sql;
    sql << "UPDATE ai3d_tasks SET downloadCount = COALESCE(downloadCount,0) + 1, update_time=NOW() WHERE tx_job_id='" << eJobId << "'";
    
    return conn.executeUpdate(sql.str());
}

// 新增：检查用户是否已下载过该模型
bool hasUserDownloadedModel(const std::string& userId, const std::string& jobId)
{
    ScopedConnection conn;
    if (!conn.isValid()) {
        return false;
    }
    
    std::string eUserId = conn.escapeString(userId);
    std::string eJobId = conn.escapeString(jobId);
    
    std::ostringstream sql;
    sql << "SELECT 1 FROM user_model_downloads WHERE user_id='" << eUserId << "' AND job_id='" << eJobId << "' LIMIT 1";
    
    auto res = conn.executeQuery(sql.str());
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res.get());
        return (row != nullptr);
    }
    return false;
}

// 新增：记录用户下载并增加模型下载计数（事务安全）
bool recordUserDownloadAndIncrementCount(const std::string& userId, const std::string& jobId)
{
    return executeWithRetry([&]() -> bool {
        ScopedConnection baseConn;
        if (!baseConn.isValid()) {
            return false;
        }
        
        TransactionManager transaction(std::move(baseConn));
        if (!transaction.begin(IsolationLevel::REPEATABLE_READ)) {
            return false;
        }
        
        try {
            std::string eUserId = transaction.escapeString(userId);
            std::string eJobId = transaction.escapeString(jobId);
            
            // 检查是否已经下载过
            std::ostringstream checkSql;
            checkSql << "SELECT 1 FROM user_model_downloads WHERE user_id='" << eUserId 
                     << "' AND job_id='" << eJobId << "' FOR UPDATE";
            
            auto checkRes = transaction.executeQuery(checkSql.str());
            if (checkRes) {
                MYSQL_ROW row = mysql_fetch_row(checkRes.get());
                if (row) {
                    // 已经下载过，不重复计数
                    return transaction.commit();
                }
            }
            
            // 插入下载记录
            std::ostringstream insertSql;
            insertSql << "INSERT INTO user_model_downloads (user_id, job_id, download_time) VALUES ('"
                      << eUserId << "', '" << eJobId << "', NOW())";
            
            if (!transaction.executeUpdate(insertSql.str())) {
                transaction.rollback();
                return false;
            }
            
            // 增加模型下载计数
            std::ostringstream updateSql;
            updateSql << "UPDATE ai3d_tasks SET downloadCount = COALESCE(downloadCount,0) + 1, update_time=NOW() WHERE tx_job_id='" << eJobId << "'";
            
            if (!transaction.executeUpdate(updateSql.str())) {
                transaction.rollback();
                return false;
            }
            
            return transaction.commit();
        } catch (const std::exception& e) {
            std::cerr << "记录用户下载异常: " << e.what() << std::endl;
            transaction.rollback();
            return false;
        }
    });
}

bool incrementModelLikeCount(const std::string &jobId)
{
    ScopedConnection conn;
    if (!conn.isValid()) {
        return false;
    }
    
    std::string eJobId = conn.escapeString(jobId);
    
    std::ostringstream sql;
    sql << "UPDATE ai3d_tasks SET `like` = COALESCE(`like`,0) + 1, update_time=NOW() WHERE tx_job_id='" << eJobId << "'";
    
    return conn.executeUpdate(sql.str());
}

/**
 * 事务内执行：递增viewCount并查询最新统计信息
 * @param conn 数据库连接（事务内）
 * @param jobId 任务ID
 * @param outStats 输出的统计信息（包含prompt）
 * @return 操作是否成功
 */
bool incrementViewAndGetStats(TransactionManager& transaction, const std::string& jobId, Json::Value& outStats) {
    try {
        std::string eJobId = transaction.escapeString(jobId);

        // 1. 递增viewCount（加行锁，确保并发安全）
        std::ostringstream updateSql;
        updateSql << "UPDATE ai3d_tasks "
                  << "SET viewCount = COALESCE(viewCount, 0) + 1, update_time = NOW() "
                  << "WHERE tx_job_id = '" << eJobId << "'"; // 行锁防止并发更新

        if (!transaction.executeUpdate(updateSql.str())) {
            return false;
        }

        // 2. 更新每日浏览量记录
        std::ostringstream dailyViewSql;
        dailyViewSql << "INSERT INTO daily_model_views (view_date, total_views) "
                     << "VALUES (CURDATE(), 1) "
                     << "ON DUPLICATE KEY UPDATE total_views = total_views + 1";

        if (!transaction.executeUpdate(dailyViewSql.str())) {
            return false;
        }

        // 3. 查询更新后的统计信息（包含prompt）
        std::ostringstream querySql;
        querySql << "SELECT "
                 << "COALESCE(`like`, 0), "
                 << "COALESCE(downloadCount, 0), "
                 << "COALESCE(viewCount, 0), "
                 << "prompt " // 新增查询prompt字段
                 << "FROM ai3d_tasks "
                 << "WHERE tx_job_id = '" << eJobId << "' LIMIT 1";

        auto res = transaction.executeQuery(querySql.str());
        if (!res) {
            return false;
        }

        MYSQL_ROW row = mysql_fetch_row(res.get());
        if (!row) {
            return false; // 任务不存在
        }

        // 解析结果到结构体
        outStats["likeCount"] = row[0] ? atoi(row[0]) : 0;
        outStats["downloadCount"] = row[1] ? atoi(row[1]) : 0;
        outStats["viewCount"] = row[2] ? atoi(row[2]) : 0;
        outStats["prompt"] = row[3] ? row[3] : ""; // 处理prompt

        return true;
    } catch (const std::exception& e) {
        std::cerr << "事务内操作失败: " << e.what() << std::endl;
        return false;
    }
}

/**
 * 对外接口：递增viewCount并获取最新统计（含事务与重试）
 * @param jobId 任务ID
 * @param outStats 输出的统计信息
 * @return 操作是否成功
 */
bool incrementModelViewAndGetStats(const std::string& jobId, Json::Value& outStats) {
    return executeWithRetry([&]() -> bool {
        ScopedConnection conn;
        if (!conn.isValid()) {
            return false;
        }

        // 开启事务（可重复读隔离级别）
        TransactionManager transaction(std::move(conn));
        if (!transaction.begin(IsolationLevel::REPEATABLE_READ)) {
            return false;
        }

        // 执行核心逻辑
        bool success = incrementViewAndGetStats(transaction, jobId, outStats);
        if (success) {
            return transaction.commit(); // 提交事务
        } else {
            transaction.rollback(); // 失败回滚
            return false;
        }
    });
}
bool getTaskStatsInternal(ScopedConnection &conn, const std::string &jobId, int &outLike, int &outDownload, int &outView)
{
    std::string eJobId = conn.escapeString(jobId);
    std::ostringstream sql;
    sql << "SELECT COALESCE(`like`,0), COALESCE(downloadCount,0), COALESCE(viewCount,0) FROM ai3d_tasks WHERE tx_job_id='" << eJobId << "' LIMIT 1";
    auto res = conn.executeQuery(sql.str());
    if (!res) return false;
    MYSQL_ROW row = mysql_fetch_row(res.get());
    if (!row) return false;
    outLike = row[0] ? atoi(row[0]) : 0;
    outDownload = row[1] ? atoi(row[1]) : 0;
    outView = row[2] ? atoi(row[2]) : 0;
    return true;
}

bool getTaskStats(const std::string &jobId, int &outLike, int &outDownload, int &outView)
{
    ScopedConnection conn;
    if (!conn.isValid()) {
        return false;
    }
    return getTaskStatsInternal(conn, jobId, outLike, outDownload, outView);
}

int getNewUserCountInRange(const std::string &startIso, const std::string &endIso)
{
    ScopedConnection conn;
    if (!conn.isValid()) {
        return -1;
    }
    std::string s = conn.escapeString(startIso);
    std::string e = conn.escapeString(endIso);
    std::ostringstream sql;
    sql << "SELECT COUNT(*) FROM users WHERE create_time >= '" << s << "' AND create_time < '" << e << "'";
    auto res = conn.executeQuery(sql.str());
    if (!res) return -1;
    MYSQL_ROW row = mysql_fetch_row(res.get());
    if (!row) return -1;
    return atoi(row[0]);
}

Json::Value getAdminOverviewStats()
{
    Json::Value out;
    ScopedConnection conn;
    if (!conn.isValid()) {
        out["ok"] = false; return out;
    }
    try {
        // 总模型数
        auto q1 = conn.executeQuery("SELECT COUNT(*) FROM ai3d_tasks");
        int totalModels = 0; if (q1) { MYSQL_ROW r = mysql_fetch_row(q1.get()); if (r) totalModels = atoi(r[0]); }
        // 至少被下载过的模型数
        auto q2 = conn.executeQuery("SELECT COUNT(*) FROM ai3d_tasks WHERE COALESCE(downloadCount,0) > 0");
        int downloadedModels = 0; if (q2) { MYSQL_ROW r = mysql_fetch_row(q2.get()); if (r) downloadedModels = atoi(r[0]); }
        // 至少被点赞过的模型数
        auto q3 = conn.executeQuery("SELECT COUNT(*) FROM ai3d_tasks WHERE COALESCE(`like`,0) > 0");
        int likedModels = 0; if (q3) { MYSQL_ROW r = mysql_fetch_row(q3.get()); if (r) likedModels = atoi(r[0]); }
        // 总用户数
        auto q4 = conn.executeQuery("SELECT COUNT(*) FROM users");
        int totalUsers = 0; if (q4) { MYSQL_ROW r = mysql_fetch_row(q4.get()); if (r) totalUsers = atoi(r[0]); }
        // 昨日新增、前日新增
        auto q5 = conn.executeQuery("SELECT COUNT(*) FROM users WHERE DATE(create_time) = CURDATE() - INTERVAL 1 DAY");
        int yesterdayNew = 0; if (q5) { MYSQL_ROW r = mysql_fetch_row(q5.get()); if (r) yesterdayNew = atoi(r[0]); }
        auto q6 = conn.executeQuery("SELECT COUNT(*) FROM users WHERE DATE(create_time) = CURDATE() - INTERVAL 2 DAY");
        int dayBeforeNew = 0; if (q6) { MYSQL_ROW r = mysql_fetch_row(q6.get()); if (r) dayBeforeNew = atoi(r[0]); }

        double totalDenom = static_cast<double>(std::max(1, totalModels));
        double downloadRate = (totalModels == 0) ? 0.0 : static_cast<double>(downloadedModels) / static_cast<double>(totalModels);
        double likeRate = (totalModels == 0) ? 0.0 : static_cast<double>(likedModels) / static_cast<double>(totalModels);
        double growthRate = 0.0;
        if (dayBeforeNew > 0) {
            growthRate = (static_cast<double>(yesterdayNew) - static_cast<double>(dayBeforeNew)) / static_cast<double>(dayBeforeNew);
        } else {
            growthRate = (yesterdayNew > 0) ? 1.0 : 0.0;
        }

        out["ok"] = true;
        out["totalModels"] = totalModels;
        out["downloadedModels"] = downloadedModels;
        out["likedModels"] = likedModels;
        out["downloadRate"] = downloadRate;
        out["likeRate"] = likeRate;
        out["totalUsers"] = totalUsers;
        out["yesterdayNewUsers"] = yesterdayNew;
        out["dayBeforeNewUsers"] = dayBeforeNew;
        out["userGrowthRate"] = growthRate;

        // 查询近14天每一天所有模型的浏览量
        auto q7 = conn.executeQuery(
            "SELECT view_date, total_views FROM daily_model_views "
            "WHERE view_date >= DATE_SUB(CURDATE(), INTERVAL 13 DAY) "
            "ORDER BY view_date ASC"
        );
        
        Json::Value dailyViews(Json::arrayValue);
        if (q7) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(q7.get())) != nullptr) {
                Json::Value dayData;
                dayData["date"] = row[0] ? row[0] : "";
                dayData["views"] = row[1] ? atoi(row[1]) : 0;
                dailyViews.append(dayData);
            }
        }
        out["dailyViews"] = dailyViews;

    } catch (...) { out["ok"] = false; }
    return out;
}

static std::string buildLikeClause(const std::string &col, const std::string &val, ScopedConnection &conn)
{
    if (val.empty()) return "";
    std::string v = conn.escapeString(val);
    return " AND " + col + " LIKE '%" + v + "%'";
}

std::pair<int, Json::Value> adminQueryUsers(const std::string &username,
                                            const std::string &email,
                                            const std::string &role,
                                            int pageNum,
                                            int pageSize)
{
    Json::Value list(Json::arrayValue); int total = 0;
    ScopedConnection conn; if (!conn.isValid()) return {0, list};
    try {
        std::ostringstream countSql; countSql << "SELECT COUNT(*) FROM users WHERE 1=1";
        countSql << buildLikeClause("username", username, conn);
        countSql << buildLikeClause("email", email, conn);
        if (!role.empty()) { std::string r = conn.escapeString(role); countSql << " AND role='" << r << "'"; }
        auto cr = conn.executeQuery(countSql.str()); if (cr) { MYSQL_ROW row = mysql_fetch_row(cr.get()); if (row) total = atoi(row[0]); }

        int startIdx = (pageNum - 1) * pageSize;
        std::ostringstream pageSql; pageSql << "SELECT user_id, username, email, role, token_count, create_time FROM users WHERE 1=1";
        pageSql << buildLikeClause("username", username, conn);
        pageSql << buildLikeClause("email", email, conn);
        if (!role.empty()) { std::string r = conn.escapeString(role); pageSql << " AND role='" << r << "'"; }
        pageSql << " ORDER BY create_time DESC LIMIT " << startIdx << ", " << pageSize;
        auto pr = conn.executeQuery(pageSql.str());
        if (pr) {
            MYSQL_ROW row; while ((row = mysql_fetch_row(pr.get())) != nullptr) {
                Json::Value item; item["userId"] = row[0] ? row[0] : ""; item["username"] = row[1] ? row[1] : ""; item["email"] = row[2] ? row[2] : ""; item["role"] = row[3] ? row[3] : ""; item["token_count"] = row[4] ? atoi(row[4]) : 0; item["create_time"] = row[5] ? row[5] : ""; list.append(item);
            }
        }
    } catch (...) { total = 0; list = Json::Value(Json::arrayValue); }
    return {total, list};
}

static void appendRange(std::ostringstream &oss, const char *col, const std::string &minV, const std::string &maxV)
{
    if (!minV.empty()) { oss << " AND COALESCE(" << col << ",0) >= " << atoi(minV.c_str()); }
    if (!maxV.empty()) { oss << " AND COALESCE(" << col << ",0) <= " << atoi(maxV.c_str()); }
}

std::pair<int, Json::Value> adminQueryModels(const std::string &minLike,
                                             const std::string &maxLike,
                                             const std::string &minDownload,
                                             const std::string &maxDownload,
                                             const std::string &isPrivate,
                                             int pageNum,
                                             int pageSize)
{
    Json::Value list(Json::arrayValue); int total = 0; ScopedConnection conn; if (!conn.isValid()) return {0, list};
    try {
        std::ostringstream base; base << " FROM ai3d_tasks WHERE 1=1";
        appendRange(base, "`like`", minLike, maxLike);
        appendRange(base, "downloadCount", minDownload, maxDownload);
        if (!isPrivate.empty()) { int v = (isPrivate == "1" || isPrivate == "true" || isPrivate == "TRUE") ? 1 : 0; base << " AND COALESCE(Isprivate,0)=" << v; }

        std::ostringstream countSql; countSql << "SELECT COUNT(*)" << base.str();
        auto cr = conn.executeQuery(countSql.str()); if (cr) { MYSQL_ROW row = mysql_fetch_row(cr.get()); if (row) total = atoi(row[0]); }

        int startIdx = (pageNum - 1) * pageSize;
        std::ostringstream pageSql; pageSql << "SELECT tx_job_id, user_id, COALESCE(`like`,0), COALESCE(downloadCount,0), COALESCE(Isprivate,0), status, result_format, version, create_time, prompt, viewCount, fileurl, previewImages" << base.str();
        pageSql << " ORDER BY create_time DESC LIMIT " << startIdx << ", " << pageSize;
        auto pr = conn.executeQuery(pageSql.str());
        if (pr) {
            MYSQL_ROW row; while ((row = mysql_fetch_row(pr.get())) != nullptr) {
                Json::Value item; item["jobId"] = row[0] ? row[0] : ""; item["userId"] = row[1] ? row[1] : ""; item["like"] = row[2] ? atoi(row[2]) : 0; item["downloadCount"] = row[3] ? atoi(row[3]) : 0; item["Isprivate"] = (row[4] && atoi(row[4]) != 0); item["status"] = row[5] ? row[5] : ""; item["resultFormat"] = row[6] ? row[6] : ""; item["version"] = row[7] ? row[7] : ""; item["create_time"] = row[8] ? row[8] : ""; item["prompt"] = row[9] ? row[9] : ""; item["viewCount"] = row[10] ? atoi(row[10]) : 0; item["fileurl"] = row[11] ? row[11] : ""; item["previewImages"] = row[12] ? row[12] : ""; list.append(item);
            }
        }
    } catch (...) { total = 0; list = Json::Value(Json::arrayValue); }
    return {total, list};
}
std::pair<int, Json::Value> queryModelsByPrivacy(bool isPrivate,
                                                 int pageNum,
                                                 int pageSize)
{
    Json::Value list(Json::arrayValue);
    int total = 0;
    
    ScopedConnection conn;
    if (!conn.isValid()) {
        std::cerr << "数据库连接失败" << std::endl;
        return {0, list};
    }

    // 统计总数
    std::ostringstream countSql;
    countSql << "SELECT COUNT(*) FROM ai3d_tasks WHERE COALESCE(Isprivate, 0) = " << (isPrivate ? 1 : 0);
    
    auto res = conn.executeQuery(countSql.str());
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res.get());
        if (row) total = atoi(row[0]);
    }

    // 分页查询
    int startIdx = (pageNum - 1) * pageSize;
    std::ostringstream pageSql;
    pageSql << "SELECT t.tx_job_id, t.user_id, t.status, t.result_format, t.version, "
        << "COALESCE(t.downloadCount,0), COALESCE(t.Isprivate,0), COALESCE(t.`like`,0), t.create_time, u.username, t.prompt, t.fileurl, t.previewImages ,t.viewCount "  // 最后添加u.username
        << "FROM ai3d_tasks t "
        << "LEFT JOIN users u ON t.user_id = u.user_id "  // 连表关联条件
        << "WHERE COALESCE(t.Isprivate, 0) = " << (isPrivate ? 1 : 0)
        << " ORDER BY t.create_time DESC LIMIT " << startIdx << ", " << pageSize;
    
    res = conn.executeQuery(pageSql.str());
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
            item["username"] = row[9] ? row[9] : "";
            item["prompt"] = row[10] ? row[10] : "";
            item["fileurl"] = row[11] ? row[11] : "";
            item["previewImages"] = row[12] ? row[12] : "";
            item["viewCount"] = row[13] ? atoi(row[13]) : 0;
            list.append(item);
        }
    }
    
    return {total, list};
}

bool toggleJobIsPrivate(const std::string& jobId)
{
    ScopedConnection conn;
    if (!conn.isValid()) {
        std::cerr << "数据库连接失败" << std::endl;
        return false;
    }
    
    std::string eJobId = conn.escapeString(jobId);
    
    std::ostringstream sql;
    sql << "UPDATE ai3d_tasks SET Isprivate = NOT Isprivate WHERE tx_job_id='" << eJobId << "'";
    
    return conn.executeUpdate(sql.str());
}

Json::Value getTaskFileInfo(const std::string& jobId)
{
    Json::Value result;
    ScopedConnection conn;
    if (!conn.isValid()) {
        std::cerr << "数据库连接失败" << std::endl;
        return result;
    }
    
    std::string eJobId = conn.escapeString(jobId);
    
    std::ostringstream sql;
    sql << "SELECT fileurl, previewImages, status, prompt, result_format, version, create_time, Isprivate "
        << "FROM ai3d_tasks WHERE tx_job_id='" << eJobId << "' LIMIT 1";
    
    auto res = conn.executeQuery(sql.str());
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
            result["Isprivate"] = (row[7] && atoi(row[7]) != 0);
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
    ScopedConnection conn;
    if (!conn.isValid()) {
        std::cerr << "数据库连接失败" << std::endl;
        return result;
    }
    
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
    }
    
    return result;
}

//
// 点赞表设计（建议DDL，添加到 database_migration.sql 并执行迁移）
//
// CREATE TABLE IF NOT EXISTS user_model_likes (
//   id BIGINT PRIMARY KEY AUTO_INCREMENT,
//   user_id VARCHAR(64) NOT NULL,
//   job_id VARCHAR(64) NOT NULL,
//   create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
//   UNIQUE KEY uniq_user_job (user_id, job_id),
//   INDEX idx_user (user_id),
//   INDEX idx_job (job_id)
// ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

bool getUserLikeForJob(const std::string& userId, const std::string& jobId)
{
    ScopedConnection conn;
    if (!conn.isValid()) {
        return false;
    }
    std::string eUser = conn.escapeString(userId);
    std::string eJob = conn.escapeString(jobId);
    std::ostringstream sql;
    sql << "SELECT 1 FROM user_model_likes WHERE user_id='" << eUser << "' AND job_id='" << eJob << "' LIMIT 1";
    auto res = conn.executeQuery(sql.str());
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res.get());
        if (row) return true;
    }
    return false;
}

bool toggleUserLikeForJob(const std::string& userId, const std::string& jobId, bool &outNewStatus)
{
    return executeWithRetry([&]() -> bool {
        ScopedConnection baseConn;
        if (!baseConn.isValid()) {
            return false;
        }
        TransactionManager tx(std::move(baseConn));
        if (!tx.begin(IsolationLevel::REPEATABLE_READ)) {
            return false;
        }
        try {
            std::string eUser = tx.escapeString(userId);
            std::string eJob = tx.escapeString(jobId);

            // 读现状
            std::ostringstream q;
            q << "SELECT 1 FROM user_model_likes WHERE user_id='" << eUser << "' AND job_id='" << eJob << "' FOR UPDATE";
            auto res = tx.executeQuery(q.str());
            bool exists = false;
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res.get());
                if (row) exists = row[0];
            }

            bool ok = false;
            if (exists) {
                // 取消点赞：删除记录
                std::ostringstream d;
                d << "DELETE FROM user_model_likes WHERE user_id='" << eUser << "' AND job_id='" << eJob << "'";
                ok = tx.executeUpdate(d.str());
                std::ostringstream u;
                u << "UPDATE ai3d_tasks SET `like` = COALESCE(`like`,0) - 1, update_time=NOW() WHERE tx_job_id='" << eJob << "'";
                ok = tx.executeUpdate(u.str());
                outNewStatus = false;
            } else {
                // 点赞：插入记录
                std::ostringstream i;
                i << "INSERT INTO user_model_likes (user_id, job_id, create_time) VALUES ('"
                  << eUser << "','" << eJob << "', NOW())";
                ok = tx.executeUpdate(i.str());
                std::ostringstream u;
                u << "UPDATE ai3d_tasks SET `like` = COALESCE(`like`,0) + 1, update_time=NOW() WHERE tx_job_id='" << eJob << "'";
                ok = tx.executeUpdate(u.str());
                outNewStatus = true;
            }
            if (!ok) { tx.rollback(); return false; }
            return tx.commit();
        } catch (const std::exception& e) {
            tx.rollback();
            return false;
        }
    });
}
