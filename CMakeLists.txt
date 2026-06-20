cmake_minimum_required(VERSION 3.14)
project(dicksuck LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── 依赖查找 ──────────────────────────────
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)

# ixwebsocket：优先 CMake 模块，回退 pkg-config
find_package(ixwebsocket QUIET)
if(NOT ixwebsocket_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(IXWEBSOCKET IMPORTED_TARGET ixwebsocket)
        if(IXWEBSOCKET_FOUND)
            set(ixwebsocket_FOUND TRUE)
        endif()
    endif()
endif()

# ── 共享链接依赖（INTERFACE 库，同时供主程序和测试链接） ──
add_library(dicksuck_deps INTERFACE)
target_link_libraries(dicksuck_deps INTERFACE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Threads::Threads
    SQLite::SQLite3
)
if(ixwebsocket_FOUND)
    target_link_libraries(dicksuck_deps INTERFACE ixwebsocket::ixwebsocket)
else()
    message(WARNING "ixwebsocket not found via CMake/pkg-config. "
        "请手动指定: -DIXWEBSOCKET_INCLUDE_DIR=... -DIXWEBSOCKET_LIB=...")
    # 取消下面注释并填写实际路径：
    # target_include_directories(dicksuck PRIVATE /usr/local/include)
    # target_link_libraries(dicksuck PRIVATE /usr/local/lib/libixwebsocket.a)
endif()

# ── 共享代码（OBJECT 库，同时供主程序和测试链接） ──
add_library(dicksuck_obj OBJECT
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

target_include_directories(dicksuck_obj PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dicksuck_obj PUBLIC dicksuck_deps)

# ── 生产可执行文件 ─────────────────────────
add_executable(dicksuck
    $<TARGET_OBJECTS:dicksuck_obj>
    main.cpp
)
target_link_libraries(dicksuck PRIVATE dicksuck_deps)

# ── 测试 ──────────────────────────────────
enable_testing()
add_executable(dicksuck_test
    $<TARGET_OBJECTS:dicksuck_obj>
    test_cases.cpp
)
target_link_libraries(dicksuck_test PRIVATE dicksuck_deps)
add_test(NAME dicksuck_unit_tests COMMAND dicksuck_test)

# ── 编译选项 ──────────────────────────────
target_compile_options(dicksuck_obj PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /O2 /utf-8>
)
