#include "auth.h"
#include "config.h"
#include "connection_pool.h"

#include <openssl/sha.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace {

std::string bytesToHex(const unsigned char *data, size_t len)
{
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    }
    return oss.str();
}

std::string sha256Hex(const std::string &input)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, input.data(), input.size());
    SHA256_Final(hash, &ctx);
    return bytesToHex(hash, SHA256_DIGEST_LENGTH);
}

}

Json::Value registerUser(const std::string &username,
                         const std::string &email,
                         const std::string &plainPassword)
{
    Json::Value resp;
    ScopedConnection conn;
    if (!conn.isValid()) {
        resp["status"] = "error";
        resp["code"] = 500;
        resp["message"] = "数据库连接失败";
        return resp;
    }

    // 检查唯一性
    {
        std::string eUsername = conn.escapeString(username);
        std::string eEmail = conn.escapeString(email);
        
        std::ostringstream query;
        query << "SELECT id FROM users WHERE username='" << eUsername << "' OR email='" << eEmail << "' LIMIT 1";
        
        auto res = conn.executeQuery(query.str());
        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res.get());
            if (row != nullptr) {
                resp["status"] = "error";
                resp["code"] = 409;
                resp["message"] = "用户名或邮箱已存在";
                return resp;
            }
        }
    }

    // 生成盐与哈希
    unsigned char saltBytes[16];
    if (RAND_bytes(saltBytes, sizeof(saltBytes)) != 1) {
        resp["status"] = "error";
        resp["code"] = 500;
        resp["message"] = "生成盐失败";
        return resp;
    }
    std::string saltHex = bytesToHex(saltBytes, sizeof(saltBytes));
    std::string hashHex = sha256Hex(saltHex + plainPassword);

    // 写入数据库
    {
        std::string eUsername = conn.escapeString(username);
        std::string eEmail = conn.escapeString(email);
        std::string eHash = conn.escapeString(hashHex);
        std::string eSalt = conn.escapeString(saltHex);

        std::ostringstream insert;
        insert << "INSERT INTO users (user_id, username, email, password_hash, password_salt, status, create_time, update_time, token_count, role) VALUES ("
               << "UUID(), '" << eUsername << "', '" << eEmail << "', '" << eHash << "', '" << eSalt << "', 1, NOW(), NOW(), 20, 'user')";
        
        if (!conn.executeUpdate(insert.str())) {
            resp["status"] = "error";
            resp["code"] = 500;
            resp["message"] = "注册失败";
            return resp;
        }
    }

    resp["status"] = "success";
    resp["code"] = 200;
    resp["message"] = "注册成功";
    return resp;
}

Json::Value loginUser(const std::string &usernameOrEmail,
                      const std::string &plainPassword)
{
    Json::Value resp;
    ScopedConnection conn;
    if (!conn.isValid()) {
        resp["status"] = "error";
        resp["code"] = 500;
        resp["message"] = "数据库连接失败";
        return resp;
    }

    std::string eInput = conn.escapeString(usernameOrEmail);

    std::ostringstream query;
    query << "SELECT user_id, password_hash, password_salt, status FROM users WHERE username='" << eInput 
          << "' OR email='" << eInput << "' LIMIT 1";
    
    auto res = conn.executeQuery(query.str());
    if (!res) {
        resp["status"] = "error";
        resp["code"] = 500;
        resp["message"] = "查询用户失败";
        return resp;
    }

    MYSQL_ROW row = mysql_fetch_row(res.get());
    if (!row) {
        resp["status"] = "error";
        resp["code"] = 401;
        resp["message"] = "用户不存在或密码错误";
        return resp;
    }

    std::string userId = row[0] ? row[0] : "";
    std::string dbHash = row[1] ? row[1] : "";
    std::string dbSalt = row[2] ? row[2] : "";
    int status = row[3] ? atoi(row[3]) : 0;

    if (status != 1) {
        resp["status"] = "error";
        resp["code"] = 403;
        resp["message"] = "账号不可用";
        return resp;
    }

    std::string calcHash = sha256Hex(dbSalt + plainPassword);
    if (calcHash != dbHash) {
        resp["status"] = "error";
        resp["code"] = 401;
        resp["message"] = "用户不存在或密码错误";
        return resp;
    }

    // 优先复用未过期的会话
    std::string tokenHex;
    {
        std::ostringstream q;
        q << "SELECT session_token FROM user_sessions WHERE user_id='" << userId
          << "' AND expire_time > NOW() ORDER BY create_time DESC LIMIT 1";
        
        auto sres = conn.executeQuery(q.str());
        if (sres) {
            MYSQL_ROW srow = mysql_fetch_row(sres.get());
            if (srow && srow[0]) {
                tokenHex = srow[0];
            }
        }
    }

    // 若没有可复用会话，生成新的 session token 并写入
    if (tokenHex.empty()) {
        unsigned char tokenBytes[32];
        if (RAND_bytes(tokenBytes, sizeof(tokenBytes)) != 1) {
            resp["status"] = "error";
            resp["code"] = 500;
            resp["message"] = "生成会话失败";
            return resp;
        }
        tokenHex = bytesToHex(tokenBytes, sizeof(tokenBytes));

        std::string eToken = conn.escapeString(tokenHex);

        std::ostringstream insert;
        insert << "INSERT INTO user_sessions (user_id, session_token, expire_time, create_time, revoked) VALUES ('"
               << userId << "', '" << eToken << "', FROM_UNIXTIME(UNIX_TIMESTAMP() + " << SESSION_TTL_SECONDS << "), NOW(), 0)";
        
        if (!conn.executeUpdate(insert.str())) {
            resp["status"] = "error";
            resp["code"] = 500;
            resp["message"] = "创建会话失败";
            return resp;
        }
    }

    resp["status"] = "success";
    resp["code"] = 200;
    resp["message"] = "登录成功";
    resp["data"]["userId"] = userId;
    resp["data"]["sessionToken"] = tokenHex;
    resp["data"]["expireInSeconds"] = SESSION_TTL_SECONDS;
    return resp;
}

Json::Value logoutUser(const std::string &sessionToken)
{
    Json::Value resp;
    ScopedConnection conn;
    if (!conn.isValid()) {
        resp["status"] = "error";
        resp["code"] = 500;
        resp["message"] = "数据库连接失败";
        return resp;
    }

    if (sessionToken.empty()) {
        resp["status"] = "error";
        resp["code"] = 400;
        resp["message"] = "会话token不能为空";
        return resp;
    }

    std::string eToken = conn.escapeString(sessionToken);
    
    // 删除会话token（登出后完全清除会话）
    std::ostringstream deleteSql;
    deleteSql << "DELETE FROM user_sessions WHERE session_token='" << eToken << "'";
    
    if (!conn.executeUpdate(deleteSql.str())) {
        resp["status"] = "error";
        resp["code"] = 500;
        resp["message"] = "登出失败";
        return resp;
    }
    
    resp["status"] = "success";
    resp["code"] = 200;
    resp["message"] = "登出成功";
    return resp;
}

bool updateUserTokenCount(const std::string& userId, int delta)
{
    ScopedConnection conn;
    if (!conn.isValid()) {
        return false;
    }
    
    std::string eUserId = conn.escapeString(userId);
    
    std::ostringstream sql;
    sql << "UPDATE users SET token_count = token_count + " << delta << ", update_time = NOW() WHERE user_id='" << eUserId << "'";
    
    return conn.executeUpdate(sql.str());
}
