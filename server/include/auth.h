#pragma once

#include <string>
#include <jsoncpp/json/json.h>

// 注册用户：传入用户名、邮箱、明文密码，返回 Json 结果（成功/失败信息）
Json::Value registerUser(const std::string &username,
                         const std::string &email,
                         const std::string &plainPassword);

// 登录：传入用户名或邮箱、明文密码，返回 Json（包含 sessionToken 与过期时间等）
Json::Value loginUser(const std::string &usernameOrEmail,
                      const std::string &plainPassword);

// 登出：撤销用户会话token
Json::Value logoutUser(const std::string &sessionToken); 