#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 数据库迁移和初始化工具
class DatabaseMigrator {
public:
    static DatabaseMigrator& get();

    // 版本号（每次有重大变更时递增）
    static constexpr int CURRENT_VERSION = 2;
    static constexpr const char* VERSION_KEY = "__db_version__";

    // 执行所有必要的迁移
    void migrate_all();

    // 检查数据库版本
    int get_db_version() const;

    // 设置数据库版本
    void set_db_version(int version);

    // 从旧格式迁移到新格式
    void migrate_jargons_schema();
    void migrate_embedding_stores();
    void migrate_config();

    // 初始化缺失的字段
    void initialize_missing_fields();

    // 备份原始数据
    void backup_original_data();

    // 数据验证
    struct ValidationReport {
        bool is_valid = true;
        std::vector<std::string> issues;
        std::vector<std::string> warnings;
        std::string timestamp;
    };

    ValidationReport validate_all_data();

    // 修复数据问题
    void repair_data();

private:
    DatabaseMigrator() = default;

    int _current_version = CURRENT_VERSION;

    // 具体的迁移函数
    void _migrate_v1_to_v2();
    void _upgrade_jargon_entry(json& entry);
    void _add_default_fields(json& entry);
};
