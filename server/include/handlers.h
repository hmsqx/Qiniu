#pragma once

#include "comm/httplib.h"

void handleGetModel(const httplib::Request &req, httplib::Response &res);
void handleQueryJobsByPage(const httplib::Request &req, httplib::Response &res);
// 异步版，使用线程池
void handleQueryJobsByPageAsync(const httplib::Request &req, httplib::Response &res);

void handleRegister(const httplib::Request &req, httplib::Response &res);
void handleLogin(const httplib::Request &req, httplib::Response &res);
void handleLogout(const httplib::Request &req, httplib::Response &res);

// 获取当前用户信息（通过请求头 Session-Token）
void handleMe(const httplib::Request &req, httplib::Response &res);

// 模型下载、收藏、展示
void handleDownloadModel(const httplib::Request &req, httplib::Response &res);
void handleLikeModel(const httplib::Request &req, httplib::Response &res);
void handleShowModel(const httplib::Request &req, httplib::Response &res);

void handleIncrTokenCount(const httplib::Request &req, httplib::Response &res);
void handleToggleJobIsPrivate(const httplib::Request &req, httplib::Response &res);

// 获取任务的详细文件信息
void handleGetTaskFiles(const httplib::Request &req, httplib::Response &res);

// 点赞相关 API
void handleGetUserLike(const httplib::Request &req, httplib::Response &res);
void handleToggleUserLike(const httplib::Request &req, httplib::Response &res);

// 浏览与比率、用户增长
void handleIncrementViewAndGetRates(const httplib::Request &req, httplib::Response &res);
void handleGetLikeRate(const httplib::Request &req, httplib::Response &res);
void handleGetDownloadRate(const httplib::Request &req, httplib::Response &res);
void handleGetUserGrowth(const httplib::Request &req, httplib::Response &res);

// 管理员端接口
void handleAdminOverview(const httplib::Request &req, httplib::Response &res);
void handleAdminQueryUsers(const httplib::Request &req, httplib::Response &res);
void handleAdminQueryModels(const httplib::Request &req, httplib::Response &res);

