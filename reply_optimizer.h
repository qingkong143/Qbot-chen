#pragma once
#include <string>
#include <vector>
#include <map>

// 回复质量优化器
class ReplyOptimizer {
public:
    static ReplyOptimizer& get();

    // 根据群风格优化回复长度
    std::string optimizeLength(const std::string& reply, const std::string& groupStyle, int maxLength = 200);

    // 根据用户风格调整语气
    std::string adjustTone(const std::string& reply, const std::string& userStyle);

    // 检测并移除敏感内容
    std::string filterSensitive(const std::string& reply);

    // 添加表情符号（基于群风格）
    std::string addEmoji(const std::string& reply, const std::string& emojiPreference);

    // 检测回复质量分数
    double scoreQuality(const std::string& reply, const std::string& userMessage);

private:
    ReplyOptimizer() = default;

    // 统计常见的低质量模式
    struct QualityIssue {
        std::string pattern;
        double penalty;
    };

    std::vector<QualityIssue> _lowQualityPatterns = {
        {"^(我不知道|不清楚|不太明白)$", 0.3},
        {"^(呃|嗯|额)$", 0.2},
        {"(\.\.\.)+", 0.15},
        {"(重复内容)", 0.25}
    };

    // 自动断句（按逗号、句号）
    std::vector<std::string> splitSentences(const std::string& text) const;

    // 计算句子相关性
    double sentenceRelevance(const std::string& sentence, const std::string& userMessage) const;
};
