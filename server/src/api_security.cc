#include "api_security.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <regex>
#include <iostream>

// IPRateLimiter 实现
IPRateLimiter::IPRateLimiter(const RateLimitConfig& config) : config_(config) {
}

void IPRateLimiter::updateConfig(const RateLimitConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    counters_.clear();
}

bool IPRateLimiter::allowRequest(const std::string& ip) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto& counter = counters_[ip];
    
    // 检查是否在时间窗口内
    if (!isInWindow(counter)) {
        resetCounter(counter);
    }
    
    // 检查请求限制
    if (counter.requests.load() >= config_.maxRequestsPerMinute) {
        return false;
    }
    
    // 增加请求计数
    counter.requests++;
    counter.lastRequest = now;
    
    return true;
}

void IPRateLimiter::cleanupExpired() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto it = counters_.begin();
    
    while (it != counters_.end()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.lastRequest).count();
        
        if (elapsed > config_.windowSizeSeconds * 2) {
            it = counters_.erase(it);
        } else {
            ++it;
        }
    }
}

IPRateLimiter::IPStats IPRateLimiter::getIPStats(const std::string& ip) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    IPStats stats;
    auto it = counters_.find(ip);
    
    if (it != counters_.end()) {
        stats.currentRequests = it->second.requests.load();
        stats.lastRequest = it->second.lastRequest;
        stats.isBlocked = stats.currentRequests >= config_.maxRequestsPerMinute;
    } else {
        stats.currentRequests = 0;
        stats.lastRequest = std::chrono::steady_clock::now();
        stats.isBlocked = false;
    }
    
    return stats;
}

bool IPRateLimiter::isInWindow(const RequestCounter& counter) const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - counter.windowStart).count();
    
    return elapsed < config_.windowSizeSeconds;
}

void IPRateLimiter::resetCounter(RequestCounter& counter) {
    counter.requests = 0;
    counter.windowStart = std::chrono::steady_clock::now();
}

// APISecurityManager 实现
APISecurityManager& APISecurityManager::getInstance() {
    static APISecurityManager instance;
    return instance;
}

void APISecurityManager::initialize(const RateLimitConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    ipLimiter_.updateConfig(config);
    
    // 初始化用户权限
    userPermissions_["admin"] = {"*"}; // 管理员有所有权限
    userPermissions_["user"] = {"get_model", "query", "downloadModel", "like", "showModel"};
    
    std::cout << "API安全管理器初始化完成" << std::endl;
}

bool APISecurityManager::isRequestAllowed(const std::string& ip, const std::string& endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查黑名单
    auto blacklistIt = blacklist_.find(ip);
    if (blacklistIt != blacklist_.end()) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::hours>(
            now - blacklistIt->second).count();
        
        if (elapsed < 24) { // 黑名单24小时
            return false;
        } else {
            blacklist_.erase(blacklistIt);
        }
    }
    
    // 检查IP限流
    return ipLimiter_.allowRequest(ip);
}

void APISecurityManager::recordRequest(const std::string& ip, const std::string& endpoint, bool success) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!success) {
        // 记录失败的请求，如果失败次数过多，加入黑名单
        auto stats = ipLimiter_.getIPStats(ip);
        if (stats.currentRequests > config_.maxRequestsPerMinute * 0.8) {
            blacklist_[ip] = std::chrono::steady_clock::now();
        }
    }
}

std::string APISecurityManager::getClientIP(const std::string& forwardedFor, const std::string& realIP) {
    if (!realIP.empty()) {
        return realIP;
    }
    
    if (!forwardedFor.empty()) {
        // 取第一个IP（客户端真实IP）
        size_t pos = forwardedFor.find(',');
        if (pos != std::string::npos) {
            return forwardedFor.substr(0, pos);
        }
        return forwardedFor;
    }
    
    return "127.0.0.1"; // 默认本地IP
}

bool APISecurityManager::checkUserPermission(const std::string& userId, const std::string& endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = userPermissions_.find(userId);
    if (it == userPermissions_.end()) {
        return false; // 用户不存在
    }
    
    const auto& permissions = it->second;
    
    // 检查通配符权限
    if (std::find(permissions.begin(), permissions.end(), "*") != permissions.end()) {
        return true;
    }
    
    // 检查具体权限
    return std::find(permissions.begin(), permissions.end(), endpoint) != permissions.end();
}

void APISecurityManager::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);
    ipLimiter_.cleanupExpired();
}

// RequestValidator 实现
bool RequestValidator::validateJSONRequest(const std::string& body, size_t maxSize) {
    if (body.empty() || body.size() > maxSize) {
        return false;
    }
    
    // 简单的JSON格式检查
    if (body[0] != '{' || body[body.size() - 1] != '}') {
        return false;
    }
    
    return true;
}

bool RequestValidator::validateParameter(const std::string& param, const std::string& type) {
    if (param.empty()) {
        return false;
    }
    
    if (type == "int") {
        try {
            std::stoi(param);
            return true;
        } catch (...) {
            return false;
        }
    } else if (type == "float") {
        try {
            std::stof(param);
            return true;
        } catch (...) {
            return false;
        }
    } else if (type == "alphanumeric") {
        return std::all_of(param.begin(), param.end(), [](char c) {
            return std::isalnum(c);
        });
    }
    
    return true;
}

bool RequestValidator::validateFilePath(const std::string& path) {
    // 防止路径遍历攻击
    if (path.find("..") != std::string::npos) {
        return false;
    }
    
    // 防止绝对路径
    if (path[0] == '/') {
        return false;
    }
    
    return true;
}

bool RequestValidator::validateSQLInjection(const std::string& input) {
    // 简单的SQL注入检测
    std::string lowerInput = input;
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
    
    std::vector<std::string> dangerousPatterns = {
        "'", "\"", ";", "--", "/*", "*/", "xp_", "sp_", "exec", "execute",
        "select", "insert", "update", "delete", "drop", "create", "alter"
    };
    
    for (const auto& pattern : dangerousPatterns) {
        if (lowerInput.find(pattern) != std::string::npos) {
            return false;
        }
    }
    
    return true;
}

bool RequestValidator::validateXSS(const std::string& input) {
    // 简单的XSS检测
    std::vector<std::string> xssPatterns = {
        "<script", "</script>", "javascript:", "onload=", "onerror=",
        "onclick=", "onmouseover=", "onfocus=", "onblur="
    };
    
    std::string lowerInput = input;
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
    
    for (const auto& pattern : xssPatterns) {
        if (lowerInput.find(pattern) != std::string::npos) {
            return false;
        }
    }
    
    return true;
}

// SessionManager 实现
SessionManager& SessionManager::getInstance() {
    static SessionManager instance;
    return instance;
}

std::string SessionManager::createSession(const std::string& userId, int expireSeconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string sessionToken = generateSessionToken();
    auto now = std::chrono::steady_clock::now();
    
    SessionInfo info;
    info.userId = userId;
    info.expireTime = now + std::chrono::seconds(expireSeconds);
    info.lastAccess = now;
    
    sessions_[sessionToken] = info;
    
    return sessionToken;
}

bool SessionManager::validateSession(const std::string& sessionToken, std::string& userId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = sessions_.find(sessionToken);
    if (it == sessions_.end()) {
        return false;
    }
    
    auto now = std::chrono::steady_clock::now();
    if (now > it->second.expireTime) {
        sessions_.erase(it);
        return false;
    }
    
    // 更新最后访问时间
    it->second.lastAccess = now;
    userId = it->second.userId;
    
    return true;
}

bool SessionManager::revokeSession(const std::string& sessionToken) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = sessions_.find(sessionToken);
    if (it != sessions_.end()) {
        sessions_.erase(it);
        return true;
    }
    
    return false;
}

void SessionManager::cleanupExpiredSessions() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto it = sessions_.begin();
    
    while (it != sessions_.end()) {
        if (now > it->second.expireTime) {
            it = sessions_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string SessionManager::generateSessionToken() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << dis(gen);
    }
    
    return ss.str();
}

// 全局函数实现
bool initializeAPISecurity(const RateLimitConfig& config) {
    APISecurityManager::getInstance().initialize(config);
    return true;
}

APISecurityManager& getAPISecurityManager() {
    return APISecurityManager::getInstance();
}

SessionManager& getSessionManager() {
    return SessionManager::getInstance();
}
