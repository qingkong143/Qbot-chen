#pragma once
#include <string>
#include <chrono>
#include <map>
#include <mutex>
#include <climits>

struct PerformanceMetric {
    int64_t count = 0;
    int64_t total_ms = 0;
    int64_t min_ms = LLONG_MAX;
    int64_t max_ms = 0;

    double avg_ms() const { return count > 0 ? (double)total_ms / count : 0.0; }
};

class PerformanceMonitor {
public:
    static PerformanceMonitor& get();

    void recordOperation(const std::string& operation, int64_t elapsed_ms);
    PerformanceMetric getMetrics(const std::string& operation);
    std::string getReport();
    void reset();

private:
    PerformanceMonitor() = default;
    std::map<std::string, PerformanceMetric> _metrics;
    std::mutex _mutex;
};

class ScopedTimer {
public:
    ScopedTimer(const std::string& operation) : _op(operation), _start(std::chrono::system_clock::now()) {}
    ~ScopedTimer() {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - _start).count();
        PerformanceMonitor::get().recordOperation(_op, elapsed);
    }

private:
    std::string _op;
    std::chrono::system_clock::time_point _start;
};
