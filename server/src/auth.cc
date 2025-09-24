#include "auth.h"
#include "config.h"

#include <mysql/mysql.h>
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

bool openConnection(MYSQL **out)
{
    *out = mysql_init(nullptr);
    if (*out == nullptr)
    {
        std::cerr << "MySQL 初始化失败" << std::endl;
        return false;
    }
    if (mysql_real_connect(*out, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, MYSQL_PORT, nullptr, 0) == nullptr)
    {
        std::cerr << "MySQL 连接失败：" << mysql_error(*out) << std::endl;
        mysql_close(*out);
        *out = nullptr;
        return false;
    }
    if (mysql_set_character_set(*out, "utf8mb4") != 0)
    {
        std::cerr << "设置字符集失败：" << mysql_error(*out) << std::endl;
        mysql_close(*out);
        *out = nullptr;
        return false;
    }
    return true;
}

}

Json::Value registerUser(const std::string &username,
                         const std::string &email,
                         const std::string &plainPassword)
{
    Json::Value resp;
    MYSQL *conn = nullptr;
    if (!openConnection(&conn))
    {
        resp["status"] = "error";
        resp["code"] = 500;
        resp["message"] = "数据库连接失败";
        return resp;
    }

    // 检查唯一性
    {
        std::string query = "SELECT id FROM users WHERE username='" + std::string(username) + "' OR email='" + std::string(email) + "' LIMIT 1";
        if (mysql_query(conn, query.c_str()) != 0)
        {
            resp["status"] = "error";
            resp["code"] = 500;
            resp["message"] = mysql_error(conn);
            mysql_close(conn);
            return resp;
        }
        MYSQL_RES *res = mysql_store_result(conn);
        MYSQL_ROW row = res ? mysql_fetch_row(res) : nullptr;
        if (row != nullptr)
        {
            if (res) mysql_free_result(res);
            mysql_close(conn);
            resp["status"] = "error";
            resp["code"] = 409;
            resp["message"] = "用户名或邮箱已存在";
            return resp;
        }
        if (res) mysql_free_result(res);
    }

    // 生成盐与哈希
    unsigned char saltBytes[16];
    if (RAND_bytes(saltBytes, sizeof(saltBytes)) != 1)
    {
        mysql_close(conn);
        resp["status"] = "error";
        resp["code"] = 500;
        resp["message"] = "生成盐失败";
        return resp;
    }
    std::string saltHex = bytesToHex(saltBytes, sizeof(saltBytes));
    std::string hashHex = sha256Hex(saltHex + plainPassword);

    // 写入数据库
    {
        std::string escUsername, escEmail, escHash, escSalt;
        escUsername.resize(username.size() * 2 + 1);
        escEmail.resize(email.size() * 2 + 1);
        escHash.resize(hashHex.size() * 2 + 1);
        escSalt.resize(saltHex.size() * 2 + 1);
        unsigned long ulen = mysql_real_escape_string(conn, &escUsername[0], username.c_str(), username.size());
        escUsername.resize(ulen);
        unsigned long elen = mysql_real_escape_string(conn, &escEmail[0], email.c_str(), email.size());
        escEmail.resize(elen);
        unsigned long hlen = mysql_real_escape_string(conn, &escHash[0], hashHex.c_str(), hashHex.size());
        escHash.resize(hlen);
        unsigned long slen = mysql_real_escape_string(conn, &escSalt[0], saltHex.c_str(), saltHex.size());
        escSalt.resize(slen);

        std::string insert = "INSERT INTO users (user_id, username, email, password_hash, password_salt, status, create_time, update_time, token_count, role) VALUES ("
                             "UUID(), '" + escUsername + "', '" + escEmail + "', '" + escHash + "', '" + escSalt + "', 1, NOW(), NOW(), 20, 'user')";
        if (mysql_query(conn, insert.c_str()) != 0)
        {
            resp["status"] = "error";
            resp["code"] = 500;
            resp["message"] = mysql_error(conn);
            mysql_close(conn);
            return resp;
        }
    }

    mysql_close(conn);
    resp["status"] = "success";
    resp["code"] = 200;
    resp["message"] = "注册成功";
    return resp;
}

Json::Value loginUser(const std::string &usernameOrEmail,
                      const std::string &plainPassword)
{
    Json::Value resp;
    MYSQL *conn = nullptr;
    if (!openConnection(&conn))
    {
        resp["status"] = "error";
        resp["code"] = 500;
        resp["message"] = "数据库连接失败";
        return resp;
    }

    std::string esc;
    esc.resize(usernameOrEmail.size() * 2 + 1);
    unsigned long elen = mysql_real_escape_string(conn, &esc[0], usernameOrEmail.c_str(), usernameOrEmail.size());
    esc.resize(elen);

    std::string query = "SELECT user_id, password_hash, password_salt, status FROM users WHERE username='" + esc + "' OR email='" + esc + "' LIMIT 1";
    if (mysql_query(conn, query.c_str()) != 0)
    {
        resp["status"] = "error";
        resp["code"] = 500;
        resp["message"] = mysql_error(conn);
        mysql_close(conn);
        return resp;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row = res ? mysql_fetch_row(res) : nullptr;
    if (!row)
    {
        if (res) mysql_free_result(res);
        mysql_close(conn);
        resp["status"] = "error";
        resp["code"] = 401;
        resp["message"] = "用户不存在或密码错误";
        return resp;
    }

    std::string userId = row[0] ? row[0] : "";
    std::string dbHash = row[1] ? row[1] : "";
    std::string dbSalt = row[2] ? row[2] : "";
    int status = row[3] ? atoi(row[3]) : 0;
    if (res) mysql_free_result(res);

    if (status != 1)
    {
        mysql_close(conn);
        resp["status"] = "error";
        resp["code"] = 403;
        resp["message"] = "账号不可用";
        return resp;
    }

    std::string calcHash = sha256Hex(dbSalt + plainPassword);
    if (calcHash != dbHash)
    {
        mysql_close(conn);
        resp["status"] = "error";
        resp["code"] = 401;
        resp["message"] = "用户不存在或密码错误";
        return resp;
    }

    // 优先复用未过期且未撤销的会话
    std::string tokenHex;
    {
        std::ostringstream q;
        q << "SELECT session_token FROM user_sessions WHERE user_id='" << userId
          << "' AND revoked=0 AND expire_time > NOW() ORDER BY create_time DESC LIMIT 1";
        std::string query = q.str();
        if (mysql_query(conn, query.c_str()) == 0)
        {
            MYSQL_RES *sres = mysql_store_result(conn);
            MYSQL_ROW srow = sres ? mysql_fetch_row(sres) : nullptr;
            if (srow && srow[0])
            {
                tokenHex = srow[0];
            }
            if (sres) mysql_free_result(sres);
        }
    }

    // 若没有可复用会话，生成新的 session token 并写入
    if (tokenHex.empty())
    {
        unsigned char tokenBytes[32];
        if (RAND_bytes(tokenBytes, sizeof(tokenBytes)) != 1)
        {
            mysql_close(conn);
            resp["status"] = "error";
            resp["code"] = 500;
            resp["message"] = "生成会话失败";
            return resp;
        }
        tokenHex = bytesToHex(tokenBytes, sizeof(tokenBytes));

        std::string escToken;
        escToken.resize(tokenHex.size() * 2 + 1);
        unsigned long tlen = mysql_real_escape_string(conn, &escToken[0], tokenHex.c_str(), tokenHex.size());
        escToken.resize(tlen);

        std::ostringstream oss;
        oss << "INSERT INTO user_sessions (user_id, session_token, expire_time, create_time, revoked) VALUES ('"
            << userId << "', '" << escToken << "', FROM_UNIXTIME(UNIX_TIMESTAMP() + " << SESSION_TTL_SECONDS << "), NOW(), 0)";
        std::string insert = oss.str();
        if (mysql_query(conn, insert.c_str()) != 0)
        {
            mysql_close(conn);
            resp["status"] = "error";
            resp["code"] = 500;
            resp["message"] = mysql_error(conn);
            return resp;
        }
    }

    mysql_close(conn);
    resp["status"] = "success";
    resp["code"] = 200;
    resp["message"] = "登录成功";
    resp["data"]["userId"] = userId;
    resp["data"]["sessionToken"] = tokenHex;
    resp["data"]["expireInSeconds"] = SESSION_TTL_SECONDS;
    return resp;
}

bool updateUserTokenCount(const std::string& userId, int delta)
{
    MYSQL *conn = nullptr;
    if (!openConnection(&conn)) {
        return false;
    }
    std::string escUserId;
    escUserId.resize(userId.size() * 2 + 1);
    unsigned long ulen = mysql_real_escape_string(conn, &escUserId[0], userId.c_str(), userId.size());
    escUserId.resize(ulen);
    std::ostringstream oss;
    oss << "UPDATE users SET token_count = token_count + " << delta << ", update_time = NOW() WHERE user_id='" << escUserId << "'";
    std::string query = oss.str();
    bool ok = (mysql_query(conn, query.c_str()) == 0);
    mysql_close(conn);
    return ok;
}