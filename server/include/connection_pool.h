#pragma once

#include <mysql/mysql.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <chrono>

// 数据库连接池配置
struct PoolConfig {
    int minConnections = 5;      // 最小连接数
    int maxConnections = 20;     // 最大连接数
    int maxIdleTime = 300;       // 最大空闲时间（秒）
    int connectionTimeout = 30;  // 连接超时时间（秒）
};

// 带时间戳的数据库连接包装器
class PooledConnection {
public:
    PooledConnection(MYSQL* conn, std::chrono::steady_clock::time_point created);
    ~PooledConnection();
    
    MYSQL* getConnection() const { return connection_; }
    bool isValid() const;
    bool isExpired() const;
    void updateLastUsed();
    
private:
    MYSQL* connection_;
    std::chrono::steady_clock::time_point created_;
    std::chrono::steady_clock::time_point lastUsed_;
    static constexpr int MAX_IDLE_SECONDS = 300; // 5分钟
};

// 数据库连接池
class ConnectionPool {
public:
    static ConnectionPool& getInstance();
    
    // 初始化连接池
    bool initialize(const PoolConfig& config);
    
    // 获取连接
    std::unique_ptr<PooledConnection> getConnection();
    
    // 归还连接
    void returnConnection(std::unique_ptr<PooledConnection> conn);
    
    // 清理过期连接
    void cleanupExpiredConnections();
    
    // 获取连接池状态
    struct PoolStatus {
        int activeConnections;
        int idleConnections;
        int totalConnections;
    };
    PoolStatus getStatus() const;
    
    // 关闭连接池
    void shutdown();

private:
    ConnectionPool() = default;
    ~ConnectionPool();
    
    // 禁用拷贝构造和赋值
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    
    // 创建新连接
    MYSQL* createConnection();
    
    // 配置
    PoolConfig config_;
    
    // 连接队列
    std::queue<std::unique_ptr<PooledConnection>> availableConnections_;
    std::atomic<int> activeConnections_{0};
    
    // 同步
    mutable std::mutex poolMutex_;
    std::condition_variable poolCondition_;
    
    // 状态
    std::atomic<bool> initialized_{false};
    std::atomic<bool> shutdown_{false};
};

// RAII风格的连接管理
class ScopedConnection {
public:
    ScopedConnection();
    ~ScopedConnection();
    
    // 禁用拷贝，允许移动
    ScopedConnection(const ScopedConnection&) = delete;
    ScopedConnection& operator=(const ScopedConnection&) = delete;
    ScopedConnection(ScopedConnection&& other) noexcept;
    ScopedConnection& operator=(ScopedConnection&& other) noexcept;
    
    MYSQL* get() const { return conn_ ? conn_->getConnection() : nullptr; }
    bool isValid() const { return conn_ && conn_->isValid(); }
    
    // 执行查询
    std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)> executeQuery(const std::string& sql);
    
    // 执行更新
    bool executeUpdate(const std::string& sql);
    
    // 获取影响的行数
    my_ulonglong getAffectedRows() const;
    
    // 转义字符串
    std::string escapeString(const std::string& input);

private:
    std::unique_ptr<PooledConnection> conn_;
};

// 初始化连接池
bool initializeConnectionPool(const PoolConfig& config = PoolConfig{});

// 全局连接池实例访问
ConnectionPool& getConnectionPool();
