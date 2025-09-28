#pragma once

#include "connection_pool.h"
#include <mysql/mysql.h>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

// 事务隔离级别
enum class IsolationLevel {
    READ_UNCOMMITTED = 0,
    READ_COMMITTED = 1,
    REPEATABLE_READ = 2,
    SERIALIZABLE = 3
};

// 事务管理器
class TransactionManager {
public:
    TransactionManager(ScopedConnection&& conn);
    ~TransactionManager();
    
    // 禁用拷贝，允许移动
    TransactionManager(const TransactionManager&) = delete;
    TransactionManager& operator=(const TransactionManager&) = delete;
    TransactionManager(TransactionManager&& other) noexcept;
    TransactionManager& operator=(TransactionManager&& other) noexcept;
    
    // 开始事务
    bool begin(IsolationLevel level = IsolationLevel::REPEATABLE_READ);
    
    // 提交事务
    bool commit();
    
    // 回滚事务
    bool rollback();
    
    // 设置保存点
    bool setSavepoint(const std::string& name);
    
    // 回滚到保存点
    bool rollbackToSavepoint(const std::string& name);
    
    // 释放保存点
    bool releaseSavepoint(const std::string& name);
    
    // 获取连接
    MYSQL* getConnection() const;
    
    // 检查事务是否活跃
    bool isActive() const { return active_; }
    
    // 执行查询
    std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)> executeQuery(const std::string& sql);
    
    // 执行更新
    bool executeUpdate(const std::string& sql);
    
    // 获取影响的行数
    my_ulonglong getAffectedRows() const;
    
    // 转义字符串
    std::string escapeString(const std::string& input);

private:
    ScopedConnection conn_;
    bool active_;
    bool committed_;
};

// RAII事务包装器
template<typename Func>
class ScopedTransaction {
public:
    ScopedTransaction(Func&& func, IsolationLevel level = IsolationLevel::REPEATABLE_READ)
        : func_(std::forward<Func>(func)), level_(level) {
        beginTransaction();
    }
    
    ~ScopedTransaction() {
        if (!committed_) {
            rollbackTransaction();
        }
    }
    
    // 提交事务
    bool commit() {
        if (!committed_) {
            committed_ = transactionManager_.commit();
            if (committed_) {
                func_(); // 执行成功回调
            }
        }
        return committed_;
    }
    
    // 获取事务管理器
    TransactionManager& getTransaction() { return transactionManager_; }

private:
    void beginTransaction() {
        ScopedConnection conn;
        transactionManager_ = TransactionManager(std::move(conn));
        transactionManager_.begin(level_);
    }
    
    void rollbackTransaction() {
        transactionManager_.rollback();
    }
    
    Func func_;
    IsolationLevel level_;
    TransactionManager transactionManager_{ScopedConnection{}};
    bool committed_ = false;
};

// 辅助函数：创建事务
template<typename Func>
ScopedTransaction<Func> createTransaction(Func&& func, IsolationLevel level = IsolationLevel::REPEATABLE_READ) {
    return ScopedTransaction<Func>(std::forward<Func>(func), level);
}

// // 死锁检测和重试机制
// class DeadlockDetector {
// public:
//     static bool isDeadlockError(const std::string& error);
//     static bool shouldRetry(const std::string& error);
//     static int getMaxRetries() { return 3; }
//     static int getRetryDelayMs() { return 100; } // 毫秒
// };

// 带重试的事务执行
template<typename Func>
bool executeWithRetry(Func&& func, int maxRetries = 3) {
    for (int i = 0; i < maxRetries; ++i) {
        try {
            if (func()) {
                return true;
            }
            
            // 检查是否需要重试
            if (i < maxRetries - 1 ) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100 * (i + 1)));
                continue;
            }
            
            return false;
        } catch (const std::exception& e) {
            if (i < maxRetries - 1 ) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100 * (i + 1)));
                continue;
            }
            throw;
        }
    }
    return false;
}
