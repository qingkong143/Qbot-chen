#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <nlohmann/json.hpp>
#include <atomic>
#include <chrono>
#include <mutex>
#include <memory>
#include <sqlite3.h>

using json = nlohmann::json;

// ========== 模型测试字符串常量 ==========
constexpr std::array<std::string_view, 13> EMBEDDING_TEST_STRINGS = {
    "你是一个有用的AI助手",
    "这是一条测试消息",
    "模型一致性校验很重要",
    "向量检索系统需要保证稳定性",
    "我喜欢学习新知识",
    "机器学习很有趣",
    "向量空间中的相似度计算",
    "自然语言处理技术",
    "深度学习模型训练",
    "数据科学和人工智能",
    "计算机视觉和NLP",
    "embedding向量表示",
    "semantic search应用"
};

constexpr float EMBEDDING_SIM_THRESHOLD = 0.99f;

// ========== 模型校验结果结构 ==========
struct ModelConsistencyCheckResult {
    bool is_consistent = false;
    std::string model_name;
    std::string check_time;
    std::vector<std::string> failed_tests;
    std::string error_message;
};

// 嵌入库中的单条项
struct EmbeddingStoreItem {
    std::string hash;                    // 内容哈希（namespace-sha256）
    std::vector<float> embedding;        // 向量数据
    std::string content;                 // 原始文本
    int64_t timestamp = 0;               // 存入时间戳
    int frequency = 1;                   // 出现频率
};

// 嵌入库（单个命名空间）
class EmbeddingStore {
public:
    EmbeddingStore(const std::string& namespace_name, const std::string& dir_path);
    ~EmbeddingStore();

    // 批量存入字符串（自动调用 embedding API）
    void batch_insert_strs(const std::vector<std::string>& strs);

    // 搜索最相似的 k 个项（返回 hash 和相似度）
    // 当库 > 1000 条时自动使用 Faiss 加速搜索
    std::vector<std::pair<std::string, float>> search_top_k(const std::vector<float>& query, int k);

    // 获取指定 hash 的内容
    std::string get_content(const std::string& hash) const;

    // 请求保存到文件（可能延迟到批量 flush）
    void request_save();

    // 立即保存到文件（JSON 格式）
    void save_to_file();

    // 批量 flush 待保存改动
    void flush_pending_saves();

    // 从文件加载
    void load_from_file();

    // 获取库大小
    size_t size() const;

    // 删除指定项
    void delete_item(const std::string& hash);

    // 清空库
    void clear();

    // 设置最大容量（超出时自动淘汰最旧条目，0=不限制）
    void set_max_size(size_t max_size) { _max_size = max_size; }
    size_t get_max_size() const { return _max_size; }

    // 重建 Faiss 索引（当库数据变更后调用）
    void rebuild_faiss_index();

    // 检查是否需要重建索引
    bool needs_rebuild() const;

    // ========== 新增：模型一致性校验接口 ==========
    /**
     * 检查嵌入模型一致性
     * @return 校验结果
     */
    ModelConsistencyCheckResult check_embedding_model_consistency();

    /**
     * 保存测试向量到本地
     */
    void save_embedding_test_vectors();

    /**
     * 加载本地测试向量
     */
    std::unordered_map<std::string, std::vector<float>> load_embedding_test_vectors();

    /**
     * 计算两个向量的余弦相似度
     */
    static float cosine_similarity(const std::vector<float>& a, const std::vector<float>& b);

private:
    std::string _namespace;
    std::string _dir_path;
    std::string _data_file;          // 旧 JSON 文件（迁移备用）
    std::string _db_file;            // 当前 SQLite 文件
    std::string _index_file;
    std::string _idx2hash_file;
    std::string _test_vectors_file;  // 新增：测试向量文件路径
    size_t _max_size = 0;            // 最大容量（0=不限制）

    // 内存存储：hash -> EmbeddingStoreItem
    std::unordered_map<std::string, EmbeddingStoreItem> _store;
    std::unordered_map<std::string, std::vector<float>> _test_vectors;  // 新增：缓存的测试向量
    mutable std::mutex _mutex;
    std::atomic<bool> _save_pending{false};
    std::chrono::steady_clock::time_point _last_save_request{};
    static constexpr std::chrono::seconds SAVE_DEBOUNCE_WINDOW{3};

    // Faiss 索引优化（当库 > 1000 条时启用）
    void* _faiss_index = nullptr;  // faiss::IndexFlatIP* (void* 避免 Faiss 头文件依赖)
    std::unordered_map<int, std::string> _idx2hash;  // Faiss 索引位置 -> hash 映射
    bool _dirty = false;  // 标记是否需要重建索引

    static constexpr size_t FAISS_THRESHOLD = 1000;  // 当库 > 1000 条时启用 Faiss

    // 计算文本哈希
    static std::string _compute_hash(const std::string& namespace_name, const std::string& text);

    // 计算余弦相似度
    static float _cosine_similarity(const std::vector<float>& a, const std::vector<float>& b);

    // 向量 L2 归一化
    static void _normalize_l2(std::vector<float>& vec);

    // Faiss 相关方法
    void _build_faiss_index();
    std::vector<std::pair<std::string, float>> _search_with_faiss(const std::vector<float>& query, int k);
    std::vector<std::pair<std::string, float>> _search_linear(const std::vector<float>& query, int k);

    // 容量管理：淘汰最旧条目
    void _evict_oldest();
};

// 嵌入库管理器（管理多个库）
class EmbeddingManager {
public:
    static EmbeddingManager& get();

    // 初始化（加载已有数据）
    void initialize(const std::string& data_dir);

    // 获取或创建指定类型的嵌入库
    EmbeddingStore& get_store(const std::string& type);  // "knowledge" / "jargon" / etc

    // 保存所有库
    void save_all();

    // 搜索跨所有库
    std::vector<std::pair<std::string, float>> search_all(const std::vector<float>& query, int k, const std::string& type = "");

    // ========== 新增：全局模型校验 ==========
    /**
     * 检查所有库的模型一致性
     * @return 是否所有库都通过校验
     */
    bool check_all_embedding_model_consistency();

private:
    EmbeddingManager() = default;

    std::unordered_map<std::string, std::unique_ptr<EmbeddingStore>> _stores;
    std::string _data_dir;
    mutable std::mutex _mutex;
};

