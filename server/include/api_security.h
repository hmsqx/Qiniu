#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <mutex>
#include <atomic>

// 限流配置
struct RateLimitConfig {
    int maxRequestsPerMinute = 60;     // 每分钟最大请求数
    int maxRequestsPerHour = 1000;     // 每小时最大请求数
    int maxRequestsPerDay = 10000;     // 每天最大请求数
    int burstLimit = 10;               // 突发请求限制
    int windowSizeSeconds = 60;        // 时间窗口大小（秒）
};

// 请求计数器
struct RequestCounter {
    std::atomic<int> requests{0};
    std::chrono::steady_clock::time_point windowStart;
    std::chrono::steady_clock::time_point lastRequest;
};

// IP限流器
class IPRateLimiter {
public:
    IPRateLimiter(const RateLimitConfig& config);
    
    // 更新配置并清空历史计数
    void updateConfig(const RateLimitConfig& config);
    
    // 检查是否允许请求
    bool allowRequest(const std::string& ip);
    
    // 清理过期的计数器
    void cleanupExpired();
    
    // 获取IP统计信息
    struct IPStats {
        int currentRequests;
        std::chrono::steady_clock::time_point lastRequest;
        bool isBlocked;
    };
    IPStats getIPStats(const std::string& ip) const;

private:
    RateLimitConfig config_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, RequestCounter> counters_;
    
    // 检查时间窗口
    bool isInWindow(const RequestCounter& counter) const;
    
    // 重置计数器
    void resetCounter(RequestCounter& counter);
};

// API安全管理器
class APISecurityManager {
public:
    static APISecurityManager& getInstance();
    
    // 初始化安全配置
    void initialize(const RateLimitConfig& config);
    
    // 检查请求是否安全
    bool isRequestAllowed(const std::string& ip, const std::string& endpoint);
    
    // 记录请求
    void recordRequest(const std::string& ip, const std::string& endpoint, bool success);
    
    // 获取客户端IP（从请求头或连接信息）
    std::string getClientIP(const std::string& forwardedFor, const std::string& realIP);
    
    // 检查用户权限
    bool checkUserPermission(const std::string& userId, const std::string& endpoint);
    
    // 清理过期数据
    void cleanup();

private:
    APISecurityManager() : ipLimiter_(RateLimitConfig{}) {}
    
    RateLimitConfig config_;
    IPRateLimiter ipLimiter_;
    mutable std::mutex mutex_;
    
    // 用户权限映射
    std::unordered_map<std::string, std::vector<std::string>> userPermissions_;
    
    // 恶意IP黑名单
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> blacklist_;
};

// 请求验证器
class RequestValidator {
public:
    // 验证JSON请求
    static bool validateJSONRequest(const std::string& body, size_t maxSize = 1024 * 1024);
    
    // 验证参数
    static bool validateParameter(const std::string& param, const std::string& type);
    
    // 验证文件路径（防止路径遍历攻击）
    static bool validateFilePath(const std::string& path);
    
    // 验证SQL注入
    static bool validateSQLInjection(const std::string& input);
    
    // 验证XSS攻击
    static bool validateXSS(const std::string& input);
};

// 会话管理器
class SessionManager {
public:
    static SessionManager& getInstance();
    
    // 创建会话
    std::string createSession(const std::string& userId, int expireSeconds = 3600);
    
    // 验证会话
    bool validateSession(const std::string& sessionToken, std::string& userId);
    
    // 撤销会话
    bool revokeSession(const std::string& sessionToken);
    
    // 清理过期会话
    void cleanupExpiredSessions();

private:
    SessionManager() = default;
    
    struct SessionInfo {
        std::string userId;
        std::chrono::steady_clock::time_point expireTime;
        std::chrono::steady_clock::time_point lastAccess;
    };
    
    mutable std::mutex mutex_;
    std::unordered_map<std::string, SessionInfo> sessions_;
    
    // 生成会话令牌
    std::string generateSessionToken();
};

// 全局安全函数
bool initializeAPISecurity(const RateLimitConfig& config = RateLimitConfig{});

// 获取安全管理器实例
APISecurityManager& getAPISecurityManager();
SessionManager& getSessionManager();
