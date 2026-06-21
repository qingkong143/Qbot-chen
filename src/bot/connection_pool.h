#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <curl/curl.h>
#include <iostream>

// CURL 连接池
class ConnectionPool {
public:
    static ConnectionPool& get();

    // 获取连接（可选等待超时）
    CURL* acquire(int timeoutMs = 1000);

    // 归还连接
    void release(CURL* curl);

    // 初始化池大小
    void initialize(int poolSize = 5);

    // 清空所有连接
    void reset();

    // 获取当前可用连接数
    int getAvailableCount() const;

    // 获取总连接数
    int getTotalCount() const;

private:
    ConnectionPool() = default;
    ~ConnectionPool();

    struct PoolConnection {
        CURL* curl = nullptr;
        bool inUse = false;
    };

    std::queue<PoolConnection> _availableConnections;
    mutable std::mutex _mutex;
    std::condition_variable _cv;
    int _totalConnections = 0;
    int _maxPoolSize = 5;

    CURL* createConnection();
};
