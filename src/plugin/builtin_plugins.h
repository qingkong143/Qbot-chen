#pragma once

#include "plugin/plugin_base.h"
#include "plugin/embedding_plugin.h"
#include "plugin/knowledge_base_plugin.h"
#include "plugin/command_handler_plugin.h"
#include "plugin/long_term_memory_plugin.h"
#include "plugin/user_management_plugin.h"
#include <vector>
#include <memory>

// ──────────────────────────────────────────────────────
// 内置插件加载器
// ──────────────────────────────────────────────────────

class BuiltinPluginLoader {
public:
    static BuiltinPluginLoader& get();

    // 加载所有内置插件
    void load_all_plugins();

    // 单独加载特定插件
    void load_embedding_plugin();
    void load_knowledge_base_plugin();
    void load_command_handler_plugin();
    void load_long_term_memory_plugin();
    void load_user_management_plugin();

    // 获取已加载的插件列表
    std::vector<std::string> get_loaded_plugins() const;

private:
    BuiltinPluginLoader() = default;
    std::vector<std::string> _loaded_plugins;
};
