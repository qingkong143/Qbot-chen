#pragma once
#include <string>
#include <map>
#include <chrono>
#include <mutex>

class QueryCache {
public:
    static QueryCache& get();

    std::string queryProfile(int64_t user_id, int64_t chat_id);
    void cacheProfile(int64_t user_id, int64_t chat_id, const std::string& profile);
    void clearCache();
    void setTTL(int seconds);

private:
    QueryCache() : _ttl_seconds(300) {}

    struct CacheEntry {
        std::string value;
        std::chrono::system_clock::time_point timestamp;
    };

    std::map<std::string, CacheEntry> _cache;
    std::mutex _mutex;
    int _ttl_seconds;

    std::string makeKey(int64_t user_id, int64_t chat_id);
    bool isExpired(const CacheEntry& entry);
};
