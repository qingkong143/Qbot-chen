#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <ctime>
#include <mutex>

using json = nlohmann::json;

// 自动清理管理器
class CleanupManager {
public:
    static CleanupManager& get();

    // 启动定期清理线程
    void start_periodic_cleanup(int interval_seconds = 3600);

    // 执行清理任务
    void cleanup_expired_data();

    // 清理配置
    struct CleanupConfig {
        int ttl_days = 7;              // 数据保留天数
        int min_frequency = 2;         // 最小出现频率（低于此值的词会被清理）
        bool cleanup_low_frequency = true;  // 是否清理低频词
        bool cleanup_expired = true;   // 是否清理过期数据
    };

    void set_config(const CleanupConfig& config);
    CleanupConfig get_config() const;

    // 手动清理指定群的数据
    void cleanup_group(int64_t group_id);

    // 获取清理统计
    struct CleanupStats {
        int items_removed = 0;
        int total_size_before = 0;
        int total_size_after = 0;
        std::string timestamp;
    };

    CleanupStats get_last_stats() const;

private:
    CleanupManager() = default;

    CleanupConfig _config;
    CleanupStats _last_stats;
    mutable std::mutex _mutex;
    bool _running = false;

    // 检查数据是否过期
    static bool _is_expired(int64_t timestamp, int ttl_days);

    // 清理 embedding 库
    void _cleanup_embedding_stores();

    // 清理行话库
    void _cleanup_jargon_data();

    // 计算文件大小
    static int _get_file_size(const std::string& path);
};
