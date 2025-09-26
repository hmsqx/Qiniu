#include "connection_pool.h"
#include "config.h"

#include <iostream>
#include <chrono>
#include <thread>

// 自定义删除器
struct MySQLResultDeleter {
    void operator()(MYSQL_RES* res) {
        if (res) {
            mysql_free_result(res);
        }
    }
};

// PooledConnection 实现
PooledConnection::PooledConnection(MYSQL* conn, std::chrono::steady_clock::time_point created)
    : connection_(conn), created_(created), lastUsed_(created) {
}

PooledConnection::~PooledConnection() {
    if (connection_) {
        mysql_close(connection_);
    }
}

bool PooledConnection::isValid() const {
    if (!connection_) return false;
    
    // 检查连接是否还活着
    return mysql_ping(connection_) == 0;
}

bool PooledConnection::isExpired() const {
    auto now = std::chrono::steady_clock::now();
    auto idleTime = std::chrono::duration_cast<std::chrono::seconds>(now - lastUsed_).count();
    return idleTime > MAX_IDLE_SECONDS;
}

void PooledConnection::updateLastUsed() {
    lastUsed_ = std::chrono::steady_clock::now();
}

// ConnectionPool 实现
ConnectionPool& ConnectionPool::getInstance() {
    static ConnectionPool instance;
    return instance;
}

bool ConnectionPool::initialize(const PoolConfig& config) {
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    if (initialized_.load()) {
        return true;
    }
    
    config_ = config;
    
    // 创建初始连接
    for (int i = 0; i < config_.minConnections; ++i) {
        MYSQL* conn = createConnection();
        if (conn) {
            availableConnections_.emplace(
                std::make_unique<PooledConnection>(conn, std::chrono::steady_clock::now())
            );
        } else {
            std::cerr << "创建初始数据库连接失败" << std::endl;
            return false;
        }
    }
    
    initialized_.store(true);
    std::cout << "数据库连接池初始化完成，初始连接数: " << config_.minConnections << std::endl;
    
    return true;
}

MYSQL* ConnectionPool::createConnection() {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "MySQL 初始化失败" << std::endl;
        return nullptr;
    }
    
    // 设置连接超时
    unsigned int timeout = config_.connectionTimeout;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    
    if (!mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, 
                           MYSQL_DATABASE, MYSQL_PORT, nullptr, 0)) {
        std::cerr << "MySQL 连接失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return nullptr;
    }
    
    if (mysql_set_character_set(conn, "utf8mb4") != 0) {
        std::cerr << "设置字符集失败：" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return nullptr;
    }
    
    return conn;
}

std::unique_ptr<PooledConnection> ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(poolMutex_);
    
    if (shutdown_.load()) {
        return nullptr;
    }
    
    // 等待可用连接或超时
    if (!poolCondition_.wait_for(lock, std::chrono::seconds(config_.connectionTimeout),
                                [this] { 
                                    return !availableConnections_.empty() || 
                                           activeConnections_.load() < config_.maxConnections ||
                                           shutdown_.load();
                                })) {
        std::cerr << "获取数据库连接超时" << std::endl;
        return nullptr;
    }
    
    if (shutdown_.load()) {
        return nullptr;
    }
    
    // 如果有可用连接，直接返回
    if (!availableConnections_.empty()) {
        auto conn = std::move(availableConnections_.front());
        availableConnections_.pop();
        
        // 检查连接是否有效
        if (conn->isValid()) {
            conn->updateLastUsed();
            activeConnections_++;
            return conn;
        } else {
            // 连接无效，创建新连接
            MYSQL* newConn = createConnection();
            if (newConn) {
                conn = std::make_unique<PooledConnection>(newConn, std::chrono::steady_clock::now());
                activeConnections_++;
                return conn;
            }
        }
    }
    
    // 创建新连接
    if (activeConnections_.load() < config_.maxConnections) {
        MYSQL* newConn = createConnection();
        if (newConn) {
            activeConnections_++;
            return std::make_unique<PooledConnection>(newConn, std::chrono::steady_clock::now());
        }
    }
    
    return nullptr;
}

void ConnectionPool::returnConnection(std::unique_ptr<PooledConnection> conn) {
    if (!conn) return;
    
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    if (shutdown_.load()) {
        return;
    }
    
    conn->updateLastUsed();
    activeConnections_--;
    
    // 如果连接池未满且连接有效，放回池中
    if (availableConnections_.size() < config_.maxConnections && conn->isValid()) {
        availableConnections_.push(std::move(conn));
        poolCondition_.notify_one();
    }
}

void ConnectionPool::cleanupExpiredConnections() {
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    std::queue<std::unique_ptr<PooledConnection>> validConnections;
    
    while (!availableConnections_.empty()) {
        auto conn = std::move(availableConnections_.front());
        availableConnections_.pop();
        
        if (conn->isValid() && !conn->isExpired()) {
            validConnections.push(std::move(conn));
        }
    }
    
    availableConnections_ = std::move(validConnections);
}

ConnectionPool::PoolStatus ConnectionPool::getStatus() const {
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    PoolStatus status;
    status.activeConnections = activeConnections_.load();
    status.idleConnections = availableConnections_.size();
    status.totalConnections = status.activeConnections + status.idleConnections;
    
    return status;
}

void ConnectionPool::shutdown() {
    std::lock_guard<std::mutex> lock(poolMutex_);
    shutdown_.store(true);
    poolCondition_.notify_all();
    
    // 清空连接池
    while (!availableConnections_.empty()) {
        availableConnections_.pop();
    }
}

ConnectionPool::~ConnectionPool() {
    shutdown();
}

// ScopedConnection 实现
ScopedConnection::ScopedConnection() {
    conn_ = getConnectionPool().getConnection();
}

ScopedConnection::~ScopedConnection() {
    if (conn_) {
        getConnectionPool().returnConnection(std::move(conn_));
    }
}

ScopedConnection::ScopedConnection(ScopedConnection&& other) noexcept
    : conn_(std::move(other.conn_)) {
}

ScopedConnection& ScopedConnection::operator=(ScopedConnection&& other) noexcept {
    if (this != &other) {
        if (conn_) {
            getConnectionPool().returnConnection(std::move(conn_));
        }
        conn_ = std::move(other.conn_);
    }
    return *this;
}

std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)> ScopedConnection::executeQuery(const std::string& sql) {
    if (!conn_ || !conn_->isValid()) {
        return std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)>(nullptr, mysql_free_result);
    }
    
    MYSQL* mysql = conn_->getConnection();
    if (mysql_query(mysql, sql.c_str()) != 0) {
        std::cerr << "查询执行失败：" << mysql_error(mysql) << std::endl;
        return std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)>(nullptr, mysql_free_result);
    }
    
    MYSQL_RES* result = mysql_store_result(mysql);
    if (!result) {
        std::cerr << "获取查询结果失败：" << mysql_error(mysql) << std::endl;
        return std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)>(nullptr, mysql_free_result);
    }
    
    return std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)>(result, mysql_free_result);
}

bool ScopedConnection::executeUpdate(const std::string& sql) {
    if (!conn_ || !conn_->isValid()) {
        return false;
    }
    
    MYSQL* mysql = conn_->getConnection();
    if (mysql_query(mysql, sql.c_str()) != 0) {
        std::cerr << "更新执行失败：" << mysql_error(mysql) << std::endl;
        return false;
    }
    
    return true;
}

my_ulonglong ScopedConnection::getAffectedRows() const {
    if (!conn_ || !conn_->isValid()) {
        return 0;
    }
    
    return mysql_affected_rows(conn_->getConnection());
}

std::string ScopedConnection::escapeString(const std::string& input) {
    if (!conn_ || !conn_->isValid()) {
        return input;
    }
    
    std::string escaped;
    escaped.resize(input.size() * 2 + 1);
    unsigned long len = mysql_real_escape_string(conn_->getConnection(), &escaped[0], 
                                                input.c_str(), input.size());
    escaped.resize(len);
    return escaped;
}

// 全局函数实现
bool initializeConnectionPool(const PoolConfig& config) {
    return ConnectionPool::getInstance().initialize(config);
}

ConnectionPool& getConnectionPool() {
    return ConnectionPool::getInstance();
}
