#include "performance_monitor.h"
#include <fstream>
#include <sstream>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <algorithm>
#include <iostream>

// PerformanceMonitor 实现
PerformanceMonitor& PerformanceMonitor::getInstance() {
    static PerformanceMonitor instance;
    return instance;
}

void PerformanceMonitor::initialize() {
    std::cout << "性能监控器初始化完成" << std::endl;
}

void PerformanceMonitor::recordMetric(const std::string& name, MetricType type, double value) {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    
    auto& metric = metrics_[name];
    metric.name = name;
    metric.type = type;
    metric.value = value;
    metric.count += 1;
    metric.lastUpdate = std::chrono::steady_clock::now();
    metric.sumValue += value;
    
    // 更新最小值和最大值
    double currentMin = metric.minValue;
    if (value < currentMin) {
        metric.minValue = value;
    }
    
    double currentMax = metric.maxValue;
    if (value > currentMax) {
        metric.maxValue = value;
    }
}

void PerformanceMonitor::recordResponseTime(const std::string& endpoint, double responseTimeMs) {
    recordMetric("response_time_" + endpoint, MetricType::RESPONSE_TIME, responseTimeMs);
    recordMetric("total_requests", MetricType::REQUEST_COUNT, 1.0);
}

void PerformanceMonitor::recordError(const std::string& endpoint, const std::string& errorType) {
    recordMetric("error_" + endpoint + "_" + errorType, MetricType::ERROR_COUNT, 1.0);
    recordMetric("total_errors", MetricType::ERROR_COUNT, 1.0);
}

double PerformanceMonitor::getMetric(const std::string& name) {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    
    auto it = metrics_.find(name);
    if (it != metrics_.end()) {
        return it->second.value;
    }
    
    return 0.0;
}

std::unordered_map<std::string, MetricData> PerformanceMonitor::getAllMetrics() {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    return metrics_;
}

PerformanceMonitor::PerformanceReport PerformanceMonitor::getPerformanceReport() {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    
    PerformanceReport report;
    
    // 计算平均响应时间
    double totalResponseTime = 0.0;
    double maxResponseTime = 0.0;
    double minResponseTime = std::numeric_limits<double>::max();
    long long totalRequests = 0;
    
    for (const auto& pair : metrics_) {
        const auto& metric = pair.second;
        
        if (metric.type == MetricType::RESPONSE_TIME) {
            totalResponseTime += metric.sumValue;
            totalRequests += metric.count;
            
            double maxVal = metric.maxValue;
            if (maxVal > maxResponseTime) {
                maxResponseTime = maxVal;
            }
            
            double minVal = metric.minValue;
            if (minVal < minResponseTime) {
                minResponseTime = minVal;
            }
        }
    }
    
    report.avgResponseTime = totalRequests > 0 ? totalResponseTime / totalRequests : 0.0;
    report.maxResponseTime = maxResponseTime;
    report.minResponseTime = minResponseTime == std::numeric_limits<double>::max() ? 0.0 : minResponseTime;
    
    // 获取请求和错误统计
    auto totalRequestsIt = metrics_.find("total_requests");
    report.totalRequests = totalRequestsIt != metrics_.end() ? totalRequestsIt->second.count : 0;
    
    auto totalErrorsIt = metrics_.find("total_errors");
    report.totalErrors = totalErrorsIt != metrics_.end() ? totalErrorsIt->second.count : 0;
    
    report.errorRate = report.totalRequests > 0 ? 
        static_cast<double>(report.totalErrors) / report.totalRequests : 0.0;
    
    // 获取系统资源信息
    report.memoryUsageMB = getCurrentMemoryUsage();
    report.cpuUsage = getCurrentCPUUsage();
    report.threadCount = getCurrentThreadCount();
    
    // 获取连接池状态
    report.activeConnections = static_cast<int>(getMetric("active_db_connections"));
    
    // 获取下载速度
    report.downloadSpeedMBps = getMetric("download_speed_mbps");
    
    return report;
}

bool PerformanceMonitor::detectPerformanceAnomaly() {
    auto report = getPerformanceReport();
    
    return report.avgResponseTime > responseTimeThreshold_ ||
           report.errorRate > errorRateThreshold_ ||
           report.memoryUsageMB > memoryThresholdMB_ ||
           report.cpuUsage > cpuThreshold_;
}

std::vector<std::string> PerformanceMonitor::getAnomalyReport() {
    std::vector<std::string> anomalies;
    auto report = getPerformanceReport();
    
    if (report.avgResponseTime > responseTimeThreshold_) {
        anomalies.push_back("平均响应时间过高: " + std::to_string(report.avgResponseTime) + "ms");
    }
    
    if (report.errorRate > errorRateThreshold_) {
        anomalies.push_back("错误率过高: " + std::to_string(report.errorRate * 100) + "%");
    }
    
    if (report.memoryUsageMB > memoryThresholdMB_) {
        anomalies.push_back("内存使用过高: " + std::to_string(report.memoryUsageMB) + "MB");
    }
    
    if (report.cpuUsage > cpuThreshold_) {
        anomalies.push_back("CPU使用率过高: " + std::to_string(report.cpuUsage) + "%");
    }
    
    return anomalies;
}

void PerformanceMonitor::cleanup() {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto it = metrics_.begin();
    
    while (it != metrics_.end()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::hours>(
            now - it->second.lastUpdate).count();
        
        if (elapsed > 24) { // 清理24小时前的数据
            it = metrics_.erase(it);
        } else {
            ++it;
        }
    }
}

double PerformanceMonitor::getCurrentMemoryUsage() {
    std::ifstream statusFile("/proc/self/status");
    std::string line;
    
    while (std::getline(statusFile, line)) {
        if (line.find("VmRSS:") == 0) {
            std::istringstream iss(line);
            std::string label;
            long long value;
            std::string unit;
            iss >> label >> value >> unit;
            
            if (unit == "kB") {
                return value / 1024.0; // 转换为MB
            }
        }
    }
    
    return 0.0;
}

double PerformanceMonitor::getCurrentCPUUsage() {
    // 简化的CPU使用率计算
    std::ifstream statFile("/proc/stat");
    std::string line;
    
    if (std::getline(statFile, line)) {
        std::istringstream iss(line);
        std::string cpu;
        long long user, nice, system, idle;
        iss >> cpu >> user >> nice >> system >> idle;
        
        long long total = user + nice + system + idle;
        long long nonIdle = user + nice + system;
        
        return total > 0 ? (static_cast<double>(nonIdle) / total) * 100.0 : 0.0;
    }
    
    return 0.0;
}

int PerformanceMonitor::getCurrentThreadCount() {
    std::ifstream statusFile("/proc/self/status");
    std::string line;
    
    while (std::getline(statusFile, line)) {
        if (line.find("Threads:") == 0) {
            std::istringstream iss(line);
            std::string label;
            int count;
            iss >> label >> count;
            return count;
        }
    }
    
    return 0;
}

// DeadlockDetector 实现
DeadlockDetector& DeadlockDetector::getInstance() {
    static DeadlockDetector instance;
    return instance;
}

void DeadlockDetector::initialize() {
    std::cout << "死锁检测器初始化完成" << std::endl;
}

void DeadlockDetector::recordLockAcquisition(const std::string& lockName, const std::string& threadId) {
    std::lock_guard<std::mutex> lock(detectorMutex_);
    
    LockInfo& lockInfo = locks_[lockName];
    lockInfo.lockName = lockName;
    lockInfo.ownerThreadId = threadId;
    lockInfo.acquisitionTime = std::chrono::steady_clock::now();
    
    threadLocks_[threadId].push_back(lockName);
}

void DeadlockDetector::recordLockRelease(const std::string& lockName, const std::string& threadId) {
    std::lock_guard<std::mutex> lock(detectorMutex_);
    
    auto lockIt = locks_.find(lockName);
    if (lockIt != locks_.end()) {
        locks_.erase(lockIt);
    }
    
    auto threadIt = threadLocks_.find(threadId);
    if (threadIt != threadLocks_.end()) {
        auto& locks = threadIt->second;
        locks.erase(std::remove(locks.begin(), locks.end(), lockName), locks.end());
        
        if (locks.empty()) {
            threadLocks_.erase(threadIt);
        }
    }
}

bool DeadlockDetector::detectDeadlock() {
    std::lock_guard<std::mutex> lock(detectorMutex_);
    
    for (const auto& threadPair : threadLocks_) {
        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> recursionStack;
        
        if (hasCycle(threadPair.first, visited, recursionStack)) {
            return true;
        }
    }
    
    return false;
}

DeadlockDetector::DeadlockReport DeadlockDetector::getDeadlockReport() {
    std::lock_guard<std::mutex> lock(detectorMutex_);
    
    DeadlockReport report;
    report.hasDeadlock = detectDeadlock();
    report.detectionTime = std::chrono::steady_clock::now();
    
    if (report.hasDeadlock) {
        // 收集涉及的死锁信息
        for (const auto& lockPair : locks_) {
            report.involvedLocks.push_back(lockPair.first);
            report.involvedThreads.push_back(lockPair.second.ownerThreadId);
        }
        
        // 构建循环描述
        std::ostringstream cycleStream;
        cycleStream << "检测到死锁循环: ";
        for (size_t i = 0; i < report.involvedThreads.size(); ++i) {
            if (i > 0) cycleStream << " -> ";
            cycleStream << report.involvedThreads[i] << "(" << report.involvedLocks[i] << ")";
        }
        report.cycleDescription = cycleStream.str();
    }
    
    return report;
}

std::vector<std::string> DeadlockDetector::getPreventionSuggestions() {
    std::vector<std::string> suggestions;
    
    suggestions.push_back("1. 使用一致的锁顺序：所有线程按相同顺序获取锁");
    suggestions.push_back("2. 使用超时锁：避免无限期等待");
    suggestions.push_back("3. 减少锁的粒度：使用更细粒度的锁");
    suggestions.push_back("4. 使用无锁数据结构：避免锁的使用");
    suggestions.push_back("5. 定期检查死锁：使用死锁检测算法");
    
    return suggestions;
}

void DeadlockDetector::cleanup() {
    std::lock_guard<std::mutex> lock(detectorMutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto it = locks_.begin();
    
    while (it != locks_.end()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.acquisitionTime).count();
        
        if (elapsed > 300) { // 清理5分钟前的锁记录
            it = locks_.erase(it);
        } else {
            ++it;
        }
    }
}

bool DeadlockDetector::hasCycle(const std::string& startThread, 
                               std::unordered_set<std::string>& visited,
                               std::unordered_set<std::string>& recursionStack) {
    if (recursionStack.find(startThread) != recursionStack.end()) {
        return true; // 发现循环
    }
    
    if (visited.find(startThread) != visited.end()) {
        return false; // 已经访问过，没有循环
    }
    
    visited.insert(startThread);
    recursionStack.insert(startThread);
    
    // 检查当前线程持有的锁是否被其他线程等待
    auto threadIt = threadLocks_.find(startThread);
    if (threadIt != threadLocks_.end()) {
        for (const auto& lockName : threadIt->second) {
            auto lockIt = locks_.find(lockName);
            if (lockIt != locks_.end()) {
                for (const auto& waitingThread : lockIt->second.waitingThreads) {
                    if (hasCycle(waitingThread, visited, recursionStack)) {
                        return true;
                    }
                }
            }
        }
    }
    
    recursionStack.erase(startThread);
    return false;
}

std::string DeadlockDetector::getCurrentThreadId() {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}

// SystemResourceMonitor 实现
SystemResourceMonitor& SystemResourceMonitor::getInstance() {
    static SystemResourceMonitor instance;
    return instance;
}

void SystemResourceMonitor::startMonitoring() {
    if (!monitoring_.load()) {
        monitoring_.store(true);
        monitoringThread_ = std::thread(&SystemResourceMonitor::monitoringLoop, this);
        std::cout << "系统资源监控启动" << std::endl;
    }
}

void SystemResourceMonitor::stopMonitoring() {
    if (monitoring_.load()) {
        monitoring_.store(false);
        if (monitoringThread_.joinable()) {
            monitoringThread_.join();
        }
        std::cout << "系统资源监控停止" << std::endl;
    }
}

SystemResourceMonitor::SystemStatus SystemResourceMonitor::getCurrentStatus() {
    return collectSystemInfo();
}

std::vector<SystemResourceMonitor::SystemStatus> SystemResourceMonitor::getHistory(int minutes) {
    std::lock_guard<std::mutex> lock(statusMutex_);
    
    std::vector<SystemStatus> result;
    auto now = std::chrono::steady_clock::now();
    
    for (const auto& status : statusHistory_) {
        auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(
            now - status.timestamp).count();
        
        if (elapsed <= minutes) {
            result.push_back(status);
        }
    }
    
    return result;
}

bool SystemResourceMonitor::checkResourceWarnings() {
    auto status = getCurrentStatus();
    
    return status.memoryUsageMB > 1024.0 || // 1GB
           status.cpuUsage > 80.0 ||         // 80%
           status.activeConnections > 15;    // 15个连接
}

void SystemResourceMonitor::monitoringLoop() {
    while (monitoring_.load()) {
        auto status = collectSystemInfo();
        
        {
            std::lock_guard<std::mutex> lock(statusMutex_);
            statusHistory_.push_back(status);
            
            // 保持最近1小时的数据
            if (statusHistory_.size() > 60) {
                statusHistory_.erase(statusHistory_.begin());
            }
        }
        
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}

SystemResourceMonitor::SystemStatus SystemResourceMonitor::collectSystemInfo() {
    SystemStatus status;
    status.timestamp = std::chrono::steady_clock::now();
    
    // 获取内存使用情况
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    long long totalMem = 0, freeMem = 0;
    
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0) {
            std::istringstream iss(line);
            std::string label, unit;
            iss >> label >> totalMem >> unit;
        } else if (line.find("MemAvailable:") == 0) {
            std::istringstream iss(line);
            std::string label, unit;
            iss >> label >> freeMem >> unit;
            break;
        }
    }
    
    status.memoryUsageMB = (totalMem - freeMem) / 1024.0;
    
    // 获取CPU使用率
    status.cpuUsage = 0.0; // 简化实现
    
    // 获取连接数
    status.activeConnections = 0; // 需要从连接池获取
    
    // 获取线程数
    std::ifstream statusFile("/proc/self/status");
    while (std::getline(statusFile, line)) {
        if (line.find("Threads:") == 0) {
            std::istringstream iss(line);
            std::string label;
            iss >> label >> status.totalThreads;
            break;
        }
    }
    
    // 获取磁盘使用率
    status.diskUsagePercent = 0.0; // 简化实现
    
    // 获取网络使用率
    status.networkUsageMBps = 0.0; // 简化实现
    
    return status;
}

// 全局函数实现
bool initializePerformanceMonitoring() {
    PerformanceMonitor::getInstance().initialize();
    DeadlockDetector::getInstance().initialize();
    SystemResourceMonitor::getInstance().startMonitoring();
    return true;
}

PerformanceMonitor& getPerformanceMonitor() {
    return PerformanceMonitor::getInstance();
}

DeadlockDetector& getDeadlockDetector() {
    return DeadlockDetector::getInstance();
}

SystemResourceMonitor& getSystemResourceMonitor() {
    return SystemResourceMonitor::getInstance();
}
