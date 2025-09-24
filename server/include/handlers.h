#pragma once

#include "comm/httplib.h"

void handleGetModel(const httplib::Request &req, httplib::Response &res);
void handleQueryJobsByPage(const httplib::Request &req, httplib::Response &res);

void handleRegister(const httplib::Request &req, httplib::Response &res);
void handleLogin(const httplib::Request &req, httplib::Response &res);

// 获取当前用户信息（通过请求头 Session-Token）
void handleMe(const httplib::Request &req, httplib::Response &res);

// 模型下载、收藏、展示
void handleDownloadModel(const httplib::Request &req, httplib::Response &res);
void handleLikeModel(const httplib::Request &req, httplib::Response &res);
void handleShowModel(const httplib::Request &req, httplib::Response &res);

void handleIncrTokenCount(const httplib::Request &req, httplib::Response &res);
void handleToggleJobIsPrivate(const httplib::Request &req, httplib::Response &res);

