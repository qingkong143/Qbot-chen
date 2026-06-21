#pragma once
#include <string>
#include <chrono>
#include <map>
#include <mutex>
#include <cstdint>

// 内存缓存项模板
template<typename T>
struct CacheItem {
    T value;
    std::chrono::system_clock::time_point timestamp;
    int64_t accessCount = 0;
    
    bool isExpired(int expireSeconds) const {
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - timestamp).count();
        return elapsed >= expireSeconds;
    }
};

// 内存管理器（单例）
class MemoryManager {
public:
    static MemoryManager& get();

    // 获取当前内存占用（MB）
    double getMemoryUsageMB() const;

    // 获取缓存大小统计
    struct CacheStats {
        int jargonCacheSize = 0;      // 行话缓存项数
        int styleCacheSize = 0;        // 风格缓存项数
        int memoryEmbeddingSize = 0;   // 向量缓存项数
        double estimatedMemoryMB = 0.0;
    };
    CacheStats getCacheStats() const;

    // 清理过期缓存
    void cleanupExpiredCache();

    // 手动清空特定缓存
    void clearJargonCache();
    void clearStyleCache();
    void clearEmbeddingCache();

    // 启动定时清理任务（后台线程）
    void startPeriodicCleanup(int intervalSeconds = 3600);  // 默认 1 小时

    // 停止定时清理
    void stopPeriodicCleanup();

    // 获取内存告警状态
    bool isMemoryPressure() const;  // 超过 512MB 时告警

private:
    MemoryManager() = default;
    ~MemoryManager();

    mutable std::mutex _mutex;
    bool _cleanupRunning = false;

    // 获取进程内存占用（平台相关）
    double getProcessMemory() const;
};
