#include "reply_optimizer.h"
#include <algorithm>
#include <regex>
#include <iostream>

ReplyOptimizer& ReplyOptimizer::get() {
    static ReplyOptimizer instance;
    return instance;
}

std::vector<std::string> ReplyOptimizer::splitSentences(const std::string& text) const {
    std::vector<std::string> sentences;
    std::string current;

    for (char c : text) {
        current += c;
        if (c == '。' || c == ',' || c == '，' || c == '！' || c == '？') {
            if (!current.empty()) {
                sentences.push_back(current);
                current.clear();
            }
        }
    }

    if (!current.empty()) {
        sentences.push_back(current);
    }

    return sentences;
}

double ReplyOptimizer::sentenceRelevance(const std::string& sentence, const std::string& userMessage) const {
    // 简单相关性计算：共同词汇数量 / 总词汇数
    int commonWords = 0;
    int totalWords = sentence.length() / 2;  // 粗略估计

    // 这里可以集成更复杂的 NLP 分析
    return commonWords > 0 ? (double)commonWords / totalWords : 0.5;
}

std::string ReplyOptimizer::optimizeLength(const std::string& reply, const std::string& groupStyle, int maxLength) {
    if ((int)reply.length() <= maxLength) {
        return reply;
    }

    // 按句子切割，保留最相关的前 N 句
    auto sentences = splitSentences(reply);
    std::string result;

    for (const auto& sentence : sentences) {
        if ((int)(result.length() + sentence.length()) <= maxLength) {
            result += sentence;
        } else {
            break;
        }
    }

    // 如果完全被截断，至少保留一句
    if (result.empty() && !sentences.empty()) {
        result = sentences[0].substr(0, maxLength);
    }

    return result;
}

std::string ReplyOptimizer::adjustTone(const std::string& reply, const std::string& userStyle) {
    // 检测群风格中的关键词来调整语气
    if (userStyle.find("随意") != std::string::npos) {
        // 添加更随意的语气词
        return reply + "~";
    }
    if (userStyle.find("正式") != std::string::npos) {
        // 移除过于随意的表达
        std::string result = reply;
        result = std::regex_replace(result, std::regex("~"), "");
        result = std::regex_replace(result, std::regex("哈哈"), "");
        return result;
    }

    return reply;
}

std::string ReplyOptimizer::filterSensitive(const std::string& reply) {
    // 简单的敏感词过滤
    static const std::vector<std::string> sensitiveWords = {
        "政治", "宗教", "种族歧视", "暴力"
    };

    std::string result = reply;
    for (const auto& word : sensitiveWords) {
        size_t pos = 0;
        while ((pos = result.find(word, pos)) != std::string::npos) {
            result.replace(pos, word.length(), std::string(word.length(), '*'));
            pos += word.length();
        }
    }

    return result;
}

std::string ReplyOptimizer::addEmoji(const std::string& reply, const std::string& emojiPreference) {
    // 基于群风格偏好添加表情
    if (emojiPreference.empty()) return reply;

    // 示例：根据情感添加表情
    if (reply.find("开心") != std::string::npos || reply.find("哈") != std::string::npos) {
        return reply + " 😄";
    }
    if (reply.find("伤心") != std::string::npos || reply.find("难过") != std::string::npos) {
        return reply + " 😢";
    }
    if (reply.find("疑惑") != std::string::npos || reply.find("不知道") != std::string::npos) {
        return reply + " 🤔";
    }

    return reply;
}

double ReplyOptimizer::scoreQuality(const std::string& reply, const std::string& userMessage) {
    double score = 1.0;

    // 检查是否为空
    if (reply.empty()) return 0.0;

    // 检查长度（太短或太长都不好）
    if (reply.length() < 3) score -= 0.3;
    if (reply.length() > 500) score -= 0.2;

    // 检查低质量模式
    for (const auto& issue : _lowQualityPatterns) {
        if (std::regex_search(reply, std::regex(issue.pattern))) {
            score -= issue.penalty;
        }
    }

    // 检查相关性
    auto sentences = splitSentences(reply);
    double avgRelevance = 0.0;
    for (const auto& sentence : sentences) {
        avgRelevance += sentenceRelevance(sentence, userMessage);
    }
    if (!sentences.empty()) {
        avgRelevance /= sentences.size();
        score += avgRelevance * 0.2;  // 相关性最多加 0.2
    }

    return std::max(0.0, std::min(1.0, score));
}
