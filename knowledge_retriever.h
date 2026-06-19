#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 知识检索和学习模块
class KnowledgeRetriever {
public:
    static KnowledgeRetriever& get();

    // 功能 1：群聊知识库检索（用户提问时）
    // 返回相关知识片段用于注入到 LLM 提示
    std::string retrieve_knowledge(int64_t group_id, const std::string& question);

    // 功能 2：行话库学习（学习群内新词）
    // 从新消息中学习并存储行话
    void learn_jargon(int64_t group_id, const std::string& user_id,
                     const std::string& message, const std::vector<std::string>& recent_messages);

    // 功能 3：相似消息去重
    // 检测当前消息是否与历史消息过于相似
    bool is_duplicate_message(int64_t group_id, const std::string& message, float similarity_threshold = 0.85f);

    // 保存所有库到磁盘
    void save_all();

    // 清空指定群的知识库
    void clear_group_knowledge(int64_t group_id);

private:
    KnowledgeRetriever() = default;

    // 检查消息是否有效（过滤 CQ 码等）
    static bool is_valid_message(const std::string& msg);

    // 从消息中提取有用内容（去除前后空白和特殊字符）
    static std::string extract_content(const std::string& msg);

    // 获取消息的 embedding
    static std::vector<float> get_message_embedding(const std::string& msg);
};
