#pragma once
#include <ctime>
#include <string>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/time.h>
#endif

namespace Platform {
    // 获取本地时间（线程安全）
    inline struct tm* localtime_safe(const time_t* timep, struct tm* result) {
#ifdef _WIN32
        localtime_s(result, timep);
        return result;
#else
        return localtime_r(timep, result);
#endif
    }

    // 获取当前时间戳（毫秒）
    inline int64_t getCurrentTimeMs() {
#ifdef _WIN32
        return GetTickCount64();
#else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
    }

    // 睡眠（毫秒）
    inline void sleep_ms(int ms) {
#ifdef _WIN32
        Sleep(ms);
#else
        usleep(ms * 1000);
#endif
    }

    // 获取工作目录
    inline std::string getWorkDir() {
#ifdef _WIN32
        char buffer[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, buffer);
        return std::string(buffer);
#else
        char buffer[1024];
        getcwd(buffer, sizeof(buffer));
        return std::string(buffer);
#endif
    }
}
