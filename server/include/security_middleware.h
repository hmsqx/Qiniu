#pragma once

#include "comm/httplib.h"
#include "api_security.h"
#include "performance_monitor.h"

// 安全中间件包装器
class SecurityMiddleware {
public:
    // 包装处理器，添加安全检查
    static httplib::Server::Handler wrapHandler(httplib::Server::Handler handler) {
        return [handler](const httplib::Request& req, httplib::Response& res) {
            // 获取客户端IP
            std::string clientIP = getAPISecurityManager().getClientIP(
                req.get_header_value("X-Forwarded-For"),
                req.get_header_value("X-Real-IP")
            );
            
            // 检查IP限流
            std::string endpoint = req.path;
            if (!getAPISecurityManager().isRequestAllowed(clientIP, endpoint)) {
                res.status = 429;
                res.set_content("{\"error\":\"请求过于频繁，请稍后再试\"}", "application/json");
                return;
            }
            
            // 记录请求开始时间
            auto startTime = std::chrono::steady_clock::now();
            
            try {
                // 调用原始处理器
                handler(req, res);
                
                // 记录成功请求
                getAPISecurityManager().recordRequest(clientIP, endpoint, true);
                
                // 记录响应时间
                auto endTime = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
                getPerformanceMonitor().recordResponseTime(endpoint, duration.count());
                
            } catch (const std::exception& e) {
                // 记录错误请求
                getAPISecurityManager().recordRequest(clientIP, endpoint, false);
                getPerformanceMonitor().recordError(endpoint, "exception");
                
                res.status = 500;
                res.set_content("{\"error\":\"服务器内部错误\"}", "application/json");
            }
        };
    }
    
    // 包装GET处理器
    static httplib::Server::Handler wrapGetHandler(httplib::Server::Handler handler) {
        return wrapHandler(handler);
    }
    
    // 包装POST处理器
    static httplib::Server::Handler wrapPostHandler(httplib::Server::Handler handler) {
        return wrapHandler(handler);
    }
};

// 宏定义简化使用
#define SECURE_GET(server, path, handler) \
    server.Get(path, SecurityMiddleware::wrapGetHandler(handler))

#define SECURE_POST(server, path, handler) \
    server.Post(path, SecurityMiddleware::wrapPostHandler(handler))
