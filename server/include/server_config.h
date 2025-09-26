#pragma once

#include "connection_pool.h"
#include "api_security.h"
#include "thread_pool.h"
#include "concurrent_downloader.h"
#include "performance_monitor.h"
#include <iostream>

// 服务器配置结构
struct ServerConfig {
    // 线程池配置
    size_t threadPoolSize = std::thread::hardware_concurrency();
    
    // 数据库连接池配置
    PoolConfig dbConfig;
    
    // API安全配置
    RateLimitConfig securityConfig;
    
    // 并发下载配置
    int maxConcurrentDownloads = 5;
    
    // 性能监控配置
    bool enablePerformanceMonitoring = true;
    bool enableDeadlockDetection = true;
    bool enableSystemResourceMonitoring = true;
    
    // 清理配置
    int cleanupIntervalMinutes = 5;
};

// 服务器初始化器
class ServerInitializer {
public:
    static bool initialize(const ServerConfig& config = ServerConfig{}) {
        std::cout << "正在初始化服务器组件..." << std::endl;
        
        try {
            // 1. 初始化线程池
            std::cout << "初始化线程池..." << std::endl;
            initThreadPool(config.threadPoolSize);
            std::cout << "线程池初始化完成，线程数: " << config.threadPoolSize << std::endl;
            
            // 2. 初始化数据库连接池
            std::cout << "初始化数据库连接池..." << std::endl;
            if (!initializeConnectionPool(config.dbConfig)) {
                std::cerr << "数据库连接池初始化失败" << std::endl;
                return false;
            }
            std::cout << "数据库连接池初始化完成" << std::endl;
            
            // 3. 初始化API安全管理
            std::cout << "初始化API安全管理..." << std::endl;
            if (!initializeAPISecurity(config.securityConfig)) {
                std::cerr << "API安全管理初始化失败" << std::endl;
                return false;
            }
            std::cout << "API安全管理初始化完成" << std::endl;
            
            // 4. 初始化并发下载器
            std::cout << "初始化并发下载器..." << std::endl;
            if (!initializeConcurrentDownloader(config.maxConcurrentDownloads)) {
                std::cerr << "并发下载器初始化失败" << std::endl;
                return false;
            }
            std::cout << "并发下载器初始化完成，最大并发数: " << config.maxConcurrentDownloads << std::endl;
            
            // 5. 初始化性能监控
            if (config.enablePerformanceMonitoring) {
                std::cout << "初始化性能监控..." << std::endl;
                if (!initializePerformanceMonitoring()) {
                    std::cerr << "性能监控初始化失败" << std::endl;
                    return false;
                }
                std::cout << "性能监控初始化完成" << std::endl;
            }
            
            // 6. 启动后台清理线程
            std::cout << "启动后台清理线程..." << std::endl;
            std::thread cleanupThread([config]() {
                backgroundCleanup(config.cleanupIntervalMinutes);
            });
            cleanupThread.detach();
            std::cout << "后台清理线程启动完成" << std::endl;
            
            std::cout << "服务器初始化完成！" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "服务器初始化失败: " << e.what() << std::endl;
            return false;
        }
    }
    
    // 获取默认配置
    static ServerConfig getDefaultConfig() {
        ServerConfig config;
        
        // 数据库连接池默认配置
        config.dbConfig.minConnections = 5;
        config.dbConfig.maxConnections = 20;
        config.dbConfig.maxIdleTime = 300;
        config.dbConfig.connectionTimeout = 30;
        
        // API安全默认配置
        config.securityConfig.maxRequestsPerMinute = 60;
        config.securityConfig.maxRequestsPerHour = 1000;
        config.securityConfig.maxRequestsPerDay = 10000;
        config.securityConfig.burstLimit = 10;
        
        return config;
    }
    
    // 获取高并发配置
    static ServerConfig getHighConcurrencyConfig() {
        ServerConfig config;
        
        // 线程池配置
        config.threadPoolSize = std::thread::hardware_concurrency() * 2;
        
        // 数据库连接池配置
        config.dbConfig.minConnections = 10;
        config.dbConfig.maxConnections = 50;
        config.dbConfig.maxIdleTime = 600;
        config.dbConfig.connectionTimeout = 30;
        
        // API安全配置
        config.securityConfig.maxRequestsPerMinute = 120;
        config.securityConfig.maxRequestsPerHour = 2000;
        config.securityConfig.maxRequestsPerDay = 20000;
        config.securityConfig.burstLimit = 20;
        
        // 并发下载配置
        config.maxConcurrentDownloads = 10;
        
        return config;
    }
    
    // 获取开发环境配置
    static ServerConfig getDevelopmentConfig() {
        ServerConfig config;
        
        // 较小的资源使用
        config.threadPoolSize = 4;
        config.dbConfig.minConnections = 2;
        config.dbConfig.maxConnections = 10;
        config.maxConcurrentDownloads = 3;
        
        // 更宽松的限流
        config.securityConfig.maxRequestsPerMinute = 300;
        config.securityConfig.maxRequestsPerHour = 5000;
        
        // 启用所有监控
        config.enablePerformanceMonitoring = true;
        config.enableDeadlockDetection = true;
        config.enableSystemResourceMonitoring = true;
        
        return config;
    }

private:
    // 后台清理任务
    static void backgroundCleanup(int intervalMinutes) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::minutes(intervalMinutes));
            
            try {
                // 清理过期连接
                getConnectionPool().cleanupExpiredConnections();
                
                // 清理过期会话
                getSessionManager().cleanupExpiredSessions();
                
                // 清理API安全数据
                getAPISecurityManager().cleanup();
                
                // 清理性能监控数据
                getPerformanceMonitor().cleanup();
                
                // 清理死锁检测数据
                getDeadlockDetector().cleanup();
                
                std::cout << "后台清理任务执行完成" << std::endl;
                
            } catch (const std::exception& e) {
                std::cerr << "后台清理任务异常: " << e.what() << std::endl;
            }
        }
    }
};
