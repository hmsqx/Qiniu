#include "db_connection.h"
#include "config.h"

#include <iostream>
#include <memory>

// 自定义删除器，用于智能指针管理MYSQL_RES
struct MySQLResultDeleter {
    void operator()(MYSQL_RES* res) {
        if (res) {
            mysql_free_result(res);
        }
    }
};

DatabaseConnection::DatabaseConnection() : connection_(nullptr), connected_(false) {
    connection_ = mysql_init(nullptr);
    if (connection_ == nullptr) {
        std::cerr << "MySQL 初始化失败：" << mysql_error(connection_) << std::endl;
        return;
    }
    
    if (mysql_real_connect(connection_, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, 
                          MYSQL_DATABASE, MYSQL_PORT, nullptr, 0) == nullptr) {
        std::cerr << "MySQL 连接失败：" << mysql_error(connection_) << std::endl;
        mysql_close(connection_);
        connection_ = nullptr;
        return;
    }
    
    if (mysql_set_character_set(connection_, "utf8mb4") != 0) {
        std::cerr << "设置字符集失败：" << mysql_error(connection_) << std::endl;
        mysql_close(connection_);
        connection_ = nullptr;
        return;
    }
    
    connected_ = true;
}

DatabaseConnection::~DatabaseConnection() {
    if (connection_ != nullptr) {
        mysql_close(connection_);
    }
}

std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)> DatabaseConnection::executeQuery(const std::string& sql) {
    if (!connected_) {
        std::cerr << "数据库未连接，无法执行查询" << std::endl;
        return std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)>(nullptr, mysql_free_result);
    }
    
    if (mysql_query(connection_, sql.c_str()) != 0) {
        std::cerr << "查询执行失败：" << mysql_error(connection_) << std::endl;
        return std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)>(nullptr, mysql_free_result);
    }
    
    MYSQL_RES* result = mysql_store_result(connection_);
    if (result == nullptr) {
        std::cerr << "获取查询结果失败：" << mysql_error(connection_) << std::endl;
        return std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)>(nullptr, mysql_free_result);
    }
    
    return std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)>(result, mysql_free_result);
}

bool DatabaseConnection::executeUpdate(const std::string& sql) {
    if (!connected_) {
        std::cerr << "数据库未连接，无法执行更新" << std::endl;
        return false;
    }
    
    if (mysql_query(connection_, sql.c_str()) != 0) {
        std::cerr << "更新执行失败：" << mysql_error(connection_) << std::endl;
        return false;
    }
    
    return true;
}

my_ulonglong DatabaseConnection::getAffectedRows() const {
    if (!connected_) {
        return 0;
    }
    return mysql_affected_rows(connection_);
}

std::string DatabaseConnection::escapeString(const std::string& input) {
    if (!connected_) {
        return input;
    }
    
    std::string escaped;
    escaped.resize(input.size() * 2 + 1);
    unsigned long len = mysql_real_escape_string(connection_, &escaped[0], 
                                                input.c_str(), input.size());
    escaped.resize(len);
    return escaped;
}

std::unique_ptr<DatabaseConnection> createDatabaseConnection() {
    auto conn = std::make_unique<DatabaseConnection>();
    if (!conn->isConnected()) {
        return nullptr;
    }
    return conn;
}
