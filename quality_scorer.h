#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <mutex>

using json = nlohmann::json;

// 知识质量评分系统
class QualityScorer {
public:
    static QualityScorer& get();

    // 为知识项评分（0-1）
    float score_knowledge(const std::string& content, int frequency, int64_t timestamp);

    // 记录用户反馈（"helpful"/"unhelpful"）
    void record_feedback(const std::string& knowledge_hash, const std::string& feedback);

    // 获取知识的当前评分
    float get_score(const std::string& knowledge_hash) const;

    // 获取用户反馈统计
    struct FeedbackStats {
        int helpful_count = 0;
        int unhelpful_count = 0;
        float score = 0.5f;
    };

    FeedbackStats get_feedback_stats(const std::string& knowledge_hash) const;

    // 按质量排序搜索结果
    std::vector<std::pair<std::string, float>> rank_by_quality(
        const std::vector<std::pair<std::string, float>>& search_results,
        bool use_feedback = true
    );

    // 配置
    struct ScoringConfig {
        float base_score = 0.5f;           // 基础分数
        float frequency_weight = 0.3f;     // 频率权重
        float recency_weight = 0.2f;       // 新鲜度权重
        float feedback_weight = 0.5f;      // 用户反馈权重
        int freshness_days = 30;           // 认为 "新鲜" 的天数
    };

    void set_config(const ScoringConfig& config);
    ScoringConfig get_config() const;

private:
    QualityScorer() = default;

    ScoringConfig _config;
    mutable std::mutex _mutex;

    // 用户反馈记录: knowledge_hash -> FeedbackStats
    std::unordered_map<std::string, FeedbackStats> _feedback_history;

    // 计算频率得分
    static float _score_frequency(int frequency);

    // 计算新鲜度得分
    static float _score_recency(int64_t timestamp, int freshness_days);

    // 规范化相似度得分
    static float _normalize_similarity(float similarity);
};
