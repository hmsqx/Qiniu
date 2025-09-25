#pragma once

#include <mysql/mysql.h>
#include <memory>
#include <string>

// RAII风格的数据库连接管理类
class DatabaseConnection {
private:
    MYSQL* connection_;
    bool connected_;
    
public:
    DatabaseConnection();
    ~DatabaseConnection();
    
    // 禁用拷贝构造和赋值
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
    
    // 获取连接指针
    MYSQL* get() const { return connection_; }
    
    // 检查是否已连接
    bool isConnected() const { return connected_; }
    
    // 执行查询，返回结果集
    std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)> executeQuery(const std::string& sql);
    
    // 执行更新操作（INSERT, UPDATE, DELETE）
    bool executeUpdate(const std::string& sql);
    
    // 获取影响的行数
    my_ulonglong getAffectedRows() const;
    
    // 转义字符串防止SQL注入
    std::string escapeString(const std::string& input);
};

// 全局函数：创建数据库连接
std::unique_ptr<DatabaseConnection> createDatabaseConnection();
