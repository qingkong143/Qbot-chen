cmake_minimum_required(VERSION 3.14)
project(dicksuck LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── 依赖查找 ──────────────────────────────
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)

# ixwebsocket 需要 Threads 模块先加载
# apt install libixwebsocket-dev 或从源码编译
find_package(ixwebsocket QUIET)
if(NOT ixwebsocket_FOUND)
    # 尝试 pkg-config
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(IXWEBSOCKET ixwebsocket)
    endif()
endif()

# ── 可执行文件 ─────────────────────────────
add_executable(dicksuck
    agent.cpp
    jargon_miner.cpp
    cleanup_manager.cpp
    command.cpp
    config.cpp
    connection_pool.cpp
    database_migrator.cpp
    deepseek.cpp
    embedding_service.cpp
    image_ocr_service.cpp
    embedding_store.cpp
    event_bus.cpp
    jargon_data_model.cpp
    jargon_inference.cpp
    knowledge_retriever.cpp
    knowledge_sharing.cpp
    logger.cpp
    long_memory.cpp
    main.cpp
    memory.cpp
    memory_manager.cpp
    message_deduplicator.cpp
    models.cpp
    napcat_bot.cpp
    perf_monitor.cpp
    plugin_loader.cpp
    query_cache.cpp
    quality_scorer.cpp
    reply_optimizer.cpp
    style_cache.cpp
    style_learner.cpp
    vocabulary_manager.cpp
    test_cases.cpp
    tools.cpp

    # P0/P1/P2 框架
    plugin_base.cpp
    process_manager.cpp
    persistence.cpp
    embedding_plugin.cpp
    knowledge_base_plugin.cpp
    command_handler_plugin.cpp
    long_term_memory_plugin.cpp
    user_management_plugin.cpp
    builtin_plugins.cpp
    main_system.cpp
)

# ── 链接 ──────────────────────────────────
target_link_libraries(dicksuck PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

if(ixwebsocket_FOUND)
    target_link_libraries(dicksuck PRIVATE ixwebsocket::ixwebsocket)
elseif(IXWEBSOCKET_FOUND)
    target_link_libraries(dicksuck PRIVATE ${IXWEBSOCKET_LIBRARIES})
    target_include_directories(dicksuck PRIVATE ${IXWEBSOCKET_INCLUDE_DIRS})
else()
    message(WARNING "ixwebsocket not found via CMake/pkg-config. "
        "请手动指定: -DIXWEBSOCKET_INCLUDE_DIR=... -DIXWEBSOCKET_LIB=...\n"
        "替换下面的 target_include_directories 和 target_link_libraries")
    # 取消下面注释并填写实际路径：
    # target_include_directories(dicksuck PRIVATE /usr/local/include)
    # target_link_libraries(dicksuck PRIVATE /usr/local/lib/libixwebsocket.a)
endif()

target_link_libraries(dicksuck PRIVATE Threads::Threads SQLite::SQLite3)

# ── 编译选项 ──────────────────────────────
target_compile_options(dicksuck PRIVATE -Wall -Wextra -O2)
