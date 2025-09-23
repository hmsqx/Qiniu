#pragma once

#include "comm/httplib.h"

void handleGetModel(const httplib::Request &req, httplib::Response &res);
void handleQueryJobsByPage(const httplib::Request &req, httplib::Response &res);

void handleRegister(const httplib::Request &req, httplib::Response &res);
void handleLogin(const httplib::Request &req, httplib::Response &res);