#pragma once
#include <string>
#include <deque>
#include <unordered_map>
#include <ctime>
#include <regex>

/**
 * 消息去重模块
 * 功能：
 * 1. 去除连续发送的相同内容
 * 2. 过滤仅包含@符号的消息
 * 3. 支持时间窗口去重
 */
class MessageDeduplicator {
public:
    struct DeduplicatorConfig {
        int max_history;
        int time_window_seconds;
        bool filter_at_only;
        int min_content_length;

        DeduplicatorConfig()
            : max_history(100)
            , time_window_seconds(300)
            , filter_at_only(true)
            , min_content_length(2) {}
    };

    MessageDeduplicator();
    explicit MessageDeduplicator(const DeduplicatorConfig& config);

    /**
     * 检查消息是否应该被去重
     * @param message 消息内容
     * @param user_id 发送者ID
     * @return true 表示消息应该处理，false 表示应该去重
     */
    bool shouldProcess(const std::string& message, const std::string& user_id = "");

    /**
     * 添加消息到历史记录
     */
    void addMessage(const std::string& message, const std::string& user_id = "");

    /**
     * 清空历史记录
     */
    void clear();

    /**
     * 设置配置
     */
    void setConfig(const DeduplicatorConfig& config);

private:
    struct HistoryEntry {
        std::string message;
        std::string user_id;
        int64_t timestamp;
    };

    DeduplicatorConfig _config;
    std::deque<HistoryEntry> _history;

    /**
     * 检查消息是否仅包含@符号
     */
    bool isAtOnlyMessage(const std::string& message) const;

    /**
     * 清理过期的历史记录
     */
    void cleanupExpiredHistory();

    static std::string stripAtPrefix(const std::string& message);
    static std::string normalizeMessage(const std::string& message);

    /**
     * 获取当前时间戳
     */
    static int64_t getCurrentTimestamp();
};
