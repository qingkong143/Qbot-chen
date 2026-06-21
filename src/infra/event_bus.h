#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <mutex>

// 事件类型枚举
enum class EventType {
    ON_MESSAGE,         // 收到消息
    PRE_REPLY,          // 回复前
    POST_REPLY,         // 回复后
    AFTER_SEND,         // 发送后
    ON_COMMAND,         // 命令触发
    ON_ERROR            // 错误发生
};

// 事件数据结构
struct Event {
    EventType type;
    std::string tag;                     // 事件标签（便于调试）
    std::map<std::string, std::string> data;  // 事件数据（KV）
    int64_t timestamp;                   // 事件时间戳
};

// 事件处理器类型
// 返回 true 表示继续传播，false 表示中断（仅对 intercept 有效）
using EventHandler = std::function<bool(const Event&)>;

class EventBus {
public:
    static EventBus& get();

    // 同步事件处理器（可中断）
    void subscribeIntercept(EventType type, const std::string& name, EventHandler handler);

    // 异步事件处理器（fire-and-forget）
    void subscribeAsync(EventType type, const std::string& name, EventHandler handler);

    // 触发同步事件（返回是否被中断）
    bool fireIntercept(const Event& event);

    // 触发异步事件
    void fireAsync(const Event& event);

    // 移除处理器
    void unsubscribe(EventType type, const std::string& name);

    // 移除所有处理器
    void clear();

    // 获取已注册的处理器数量
    int getHandlerCount(EventType type) const;

private:
    EventBus() = default;

    struct Handler {
        std::string name;
        EventHandler func;
    };

    std::map<EventType, std::vector<Handler>> _interceptHandlers;  // 同步
    std::map<EventType, std::vector<Handler>> _asyncHandlers;      // 异步
    mutable std::mutex _mutex;
};
