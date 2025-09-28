#include "transaction_manager.h"
#include "deallock_detector.h"
#include <iostream>
#include <thread>
#include <chrono>

// TransactionManager 实现
TransactionManager::TransactionManager(ScopedConnection&& conn)
    : conn_(std::move(conn)), active_(false), committed_(false) {
}

TransactionManager::~TransactionManager() {
    if (active_ && !committed_) {
        rollback();
    }
}

TransactionManager::TransactionManager(TransactionManager&& other) noexcept
    : conn_(std::move(other.conn_)), active_(other.active_), committed_(other.committed_) {
    other.active_ = false;
    other.committed_ = false;
}

TransactionManager& TransactionManager::operator=(TransactionManager&& other) noexcept {
    if (this != &other) {
        if (active_ && !committed_) {
            rollback();
        }
        
        conn_ = std::move(other.conn_);
        active_ = other.active_;
        committed_ = other.committed_;
        
        other.active_ = false;
        other.committed_ = false;
    }
    return *this;
}

bool TransactionManager::begin(IsolationLevel level) {
    if (active_) {
        return false; // 事务已经在进行中
    }
    
    MYSQL* mysql = getConnection();
    if (!mysql) {
        return false;
    }
    
    // 设置隔离级别
    std::string isolationQuery;
    switch (level) {
        case IsolationLevel::READ_UNCOMMITTED:
            isolationQuery = "SET TRANSACTION ISOLATION LEVEL READ UNCOMMITTED";
            break;
        case IsolationLevel::READ_COMMITTED:
            isolationQuery = "SET TRANSACTION ISOLATION LEVEL READ COMMITTED";
            break;
        case IsolationLevel::REPEATABLE_READ:
            isolationQuery = "SET TRANSACTION ISOLATION LEVEL REPEATABLE READ";
            break;
        case IsolationLevel::SERIALIZABLE:
            isolationQuery = "SET TRANSACTION ISOLATION LEVEL SERIALIZABLE";
            break;
    }
    
    if (mysql_query(mysql, isolationQuery.c_str()) != 0) {
        std::cerr << "设置事务隔离级别失败：" << mysql_error(mysql) << std::endl;
        return false;
    }
    
    // 开始事务
    if (mysql_query(mysql, "START TRANSACTION") != 0) {
        std::cerr << "开始事务失败：" << mysql_error(mysql) << std::endl;
        return false;
    }
    
    active_ = true;
    committed_ = false;
    return true;
}

bool TransactionManager::commit() {
    if (!active_) {
        return false;
    }
    
    MYSQL* mysql = getConnection();
    if (!mysql) {
        return false;
    }
    
    if (mysql_query(mysql, "COMMIT") != 0) {
        std::cerr << "提交事务失败：" << mysql_error(mysql) << std::endl;
        return false;
    }
    
    active_ = false;
    committed_ = true;
    return true;
}

bool TransactionManager::rollback() {
    if (!active_) {
        return true; // 没有活跃事务，认为成功
    }
    
    MYSQL* mysql = getConnection();
    if (!mysql) {
        return false;
    }
    
    if (mysql_query(mysql, "ROLLBACK") != 0) {
        std::cerr << "回滚事务失败：" << mysql_error(mysql) << std::endl;
        return false;
    }
    
    active_ = false;
    committed_ = false;
    return true;
}

bool TransactionManager::setSavepoint(const std::string& name) {
    if (!active_) {
        return false;
    }
    
    MYSQL* mysql = getConnection();
    if (!mysql) {
        return false;
    }
    
    std::string query = "SAVEPOINT " + escapeString(name);
    if (mysql_query(mysql, query.c_str()) != 0) {
        std::cerr << "设置保存点失败：" << mysql_error(mysql) << std::endl;
        return false;
    }
    
    return true;
}

bool TransactionManager::rollbackToSavepoint(const std::string& name) {
    if (!active_) {
        return false;
    }
    
    MYSQL* mysql = getConnection();
    if (!mysql) {
        return false;
    }
    
    std::string query = "ROLLBACK TO SAVEPOINT " + escapeString(name);
    if (mysql_query(mysql, query.c_str()) != 0) {
        std::cerr << "回滚到保存点失败：" << mysql_error(mysql) << std::endl;
        return false;
    }
    
    return true;
}

bool TransactionManager::releaseSavepoint(const std::string& name) {
    if (!active_) {
        return false;
    }
    
    MYSQL* mysql = getConnection();
    if (!mysql) {
        return false;
    }
    
    std::string query = "RELEASE SAVEPOINT " + escapeString(name);
    if (mysql_query(mysql, query.c_str()) != 0) {
        std::cerr << "释放保存点失败：" << mysql_error(mysql) << std::endl;
        return false;
    }
    
    return true;
}

MYSQL* TransactionManager::getConnection() const {
    return conn_.get();
}

std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)> TransactionManager::executeQuery(const std::string& sql) {
    return conn_.executeQuery(sql);
}

bool TransactionManager::executeUpdate(const std::string& sql) {
    return conn_.executeUpdate(sql);
}

my_ulonglong TransactionManager::getAffectedRows() const {
    return conn_.getAffectedRows();
}

std::string TransactionManager::escapeString(const std::string& input) {
    return conn_.escapeString(input);
}
