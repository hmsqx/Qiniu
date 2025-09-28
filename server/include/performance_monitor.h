#pragma once


#include <chrono>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <thread>
#include <limits>


// 性能指标类型
enum class MetricType {
    REQUEST_COUNT,
    RESPONSE_TIME,
    ERROR_COUNT,
    MEMORY_USAGE,
    CPU_USAGE,
    DB_CONNECTIONS,
    THREAD_COUNT,
    DOWNLOAD_SPEED
};

// 性能指标数据
struct MetricData {
    std::string name;
    MetricType type;
    double value{0.0};
    long long count{0};
    std::chrono::steady_clock::time_point lastUpdate;
    double minValue{std::numeric_limits<double>::max()};
    double maxValue{std::numeric_limits<double>::min()};
    double sumValue{0.0};
};

// 性能监控器
class PerformanceMonitor {
public:
    static PerformanceMonitor& getInstance();
    
    // 初始化监控器
    void initialize();
    
    // 记录指标
    void recordMetric(const std::string& name, MetricType type, double value);
    
    // 记录响应时间
    void recordResponseTime(const std::string& endpoint, double responseTimeMs);
    
    // 记录错误
    void recordError(const std::string& endpoint, const std::string& errorType);
    
    // 获取指标
    double getMetric(const std::string& name);
    
    // 获取所有指标
    std::unordered_map<std::string, MetricData> getAllMetrics();
    
    // 获取性能报告
    struct PerformanceReport {
        double avgResponseTime;
        double maxResponseTime;
        double minResponseTime;
        long long totalRequests;
        long long totalErrors;
        double errorRate;
        double memoryUsageMB;
        double cpuUsage;
        int activeConnections;
        int threadCount;
        double downloadSpeedMBps;
    };
    PerformanceReport getPerformanceReport();
    
    // 检测性能异常
    bool detectPerformanceAnomaly();
    
    // 获取异常报告
    std::vector<std::string> getAnomalyReport();
    
    // 清理旧数据
    void cleanup();

private:
    PerformanceMonitor() = default;
    
    mutable std::mutex metricsMutex_;
    std::unordered_map<std::string, MetricData> metrics_;
    
    // 异常检测阈值
    double responseTimeThreshold_ = 5000.0; // 5秒
    double errorRateThreshold_ = 0.1; // 10%
    double memoryThresholdMB_ = 1024.0; // 1GB
    double cpuThreshold_ = 80.0; // 80%
    
    // 系统信息获取
    double getCurrentMemoryUsage();
    double getCurrentCPUUsage();
    int getCurrentThreadCount();
};

// 死锁检测器
class DeadlockDetector {
    public:
        static DeadlockDetector& getInstance();
        static bool shouldRetry(const std::string& error);
        static bool isDeadlockError(const std::string& error);
        // 初始化检测器
        void initialize();
        
        // 记录锁获取
        void recordLockAcquisition(const std::string& lockName, const std::string& threadId);
        
        // 记录锁释放
        void recordLockRelease(const std::string& lockName, const std::string& threadId);
        
        // 检测死锁
        bool detectDeadlock();
        
        // 获取死锁报告
        struct DeadlockReport {
            bool hasDeadlock;
            std::vector<std::string> involvedLocks;
            std::vector<std::string> involvedThreads;
            std::string cycleDescription;
            std::chrono::steady_clock::time_point detectionTime;
        };
        DeadlockReport getDeadlockReport();
        
        // 预防死锁建议
        std::vector<std::string> getPreventionSuggestions();
        
        // 清理数据
        void cleanup();
    
    private:
        DeadlockDetector() = default;
        
        struct LockInfo {
            std::string lockName;
            std::string ownerThreadId;
            std::chrono::steady_clock::time_point acquisitionTime;
            std::vector<std::string> waitingThreads;
        };
        
        mutable std::mutex detectorMutex_;
        std::unordered_map<std::string, LockInfo> locks_;
        std::unordered_map<std::string, std::vector<std::string>> threadLocks_;
        
        // 死锁检测算法
        bool hasCycle(const std::string& startThread, std::unordered_set<std::string>& visited, 
                      std::unordered_set<std::string>& recursionStack);
        
        // 获取线程ID
        std::string getCurrentThreadId();
    };
    

// 系统资源监控
class SystemResourceMonitor {
public:
    static SystemResourceMonitor& getInstance();
    
    // 开始监控
    void startMonitoring();
    
    // 停止监控
    void stopMonitoring();
    
    // 获取系统状态
    struct SystemStatus {
        double memoryUsageMB;
        double cpuUsage;
        int activeConnections;
        int totalThreads;
        double diskUsagePercent;
        double networkUsageMBps;
        std::chrono::steady_clock::time_point timestamp;
    };
    SystemStatus getCurrentStatus();
    
    // 获取历史数据
    std::vector<SystemStatus> getHistory(int minutes = 60);
    
    // 检查资源警告
    bool checkResourceWarnings();

private:
    SystemResourceMonitor() = default;
    
    std::atomic<bool> monitoring_{false};
    std::thread monitoringThread_;
    mutable std::mutex statusMutex_;
    std::vector<SystemStatus> statusHistory_;
    
    // 监控线程函数
    void monitoringLoop();
    
    // 获取系统信息
    SystemStatus collectSystemInfo();
};

// 全局函数
bool initializePerformanceMonitoring();
PerformanceMonitor& getPerformanceMonitor();
DeadlockDetector& getDeadlockDetector();
SystemResourceMonitor& getSystemResourceMonitor();
