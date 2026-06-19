#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3
};

class Logger {
public:
    static Logger& get();

    void setLevel(LogLevel level) { _level = level; }
    LogLevel getLevel() const { return _level; }

    void debug(const std::string& tag, const std::string& message);
    void info(const std::string& tag, const std::string& message);
    void warn(const std::string& tag, const std::string& message);
    void error(const std::string& tag, const std::string& message);

    // 便捷宏
    void log(LogLevel level, const std::string& tag, const std::string& message);

private:
    Logger() = default;
    LogLevel _level = LogLevel::Info;

    std::string formatTime() const;
    std::string levelToString(LogLevel level) const;
    void output(LogLevel level, const std::string& tag, const std::string& message);
};

// 便捷宏定义
#define LOG_DEBUG(tag, msg) Logger::get().debug(tag, msg)
#define LOG_INFO(tag, msg) Logger::get().info(tag, msg)
#define LOG_WARN(tag, msg) Logger::get().warn(tag, msg)
#define LOG_ERROR(tag, msg) Logger::get().error(tag, msg)
