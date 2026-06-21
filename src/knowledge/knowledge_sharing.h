#pragma once
#include <string>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>
#include <mutex>
#include <cstdint>

using json = nlohmann::json;

// 跨群知识共享管理器
class KnowledgeSharing {
public:
    static KnowledgeSharing& get();

    // 标记知识为全局可共享
    void mark_as_global(const std::string& knowledge_hash);

    // 标记知识为私有（仅限当前群）
    void mark_as_private(const std::string& knowledge_hash);

    // 检查知识是否可在目标群共享
    bool can_share_to_group(const std::string& knowledge_hash, int64_t target_group_id) const;

    // 从全局库搜索知识
    std::vector<std::pair<std::string, float>> search_global_knowledge(
        const std::vector<float>& query, int k, int64_t exclude_group_id = 0
    );

    // 获取群的知识共享设置
    struct GroupSharingSettings {
        bool allow_import = true;           // 是否允许导入全局知识
        bool allow_export = true;           // 是否允许导出本群知识到全局
        std::set<int64_t> blocked_groups;   // 黑名单群组（不共享到这些群）
    };

    void set_group_settings(int64_t group_id, const GroupSharingSettings& settings);
    GroupSharingSettings get_group_settings(int64_t group_id) const;

    // 知识隐私级别
    enum class PrivacyLevel {
        PUBLIC = 0,      // 全局可见
        GROUP = 1,       // 仅限当前群
        PRIVATE = 2      // 完全私密
    };

    void set_privacy_level(const std::string& knowledge_hash, PrivacyLevel level);
    PrivacyLevel get_privacy_level(const std::string& knowledge_hash) const;

    // 统计
    struct SharingStats {
        int global_knowledge_count = 0;      // 全局知识数量
        int private_knowledge_count = 0;     // 私有知识数量
        int total_shares = 0;                // 总共享次数
        int blocked_shares = 0;              // 被阻止的共享次数
    };

    SharingStats get_stats() const;

private:
    KnowledgeSharing() = default;

    mutable std::mutex _mutex;

    // 知识隐私标记: knowledge_hash -> PrivacyLevel
    std::unordered_map<std::string, PrivacyLevel> _privacy_levels;

    // 群组共享设置: group_id -> GroupSharingSettings
    std::unordered_map<int64_t, GroupSharingSettings> _group_settings;

    // 统计
    SharingStats _stats;
};
