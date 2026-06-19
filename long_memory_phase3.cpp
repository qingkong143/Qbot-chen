// Phase 3: 混合检索和图谱关系 - 添加到 long_memory.cpp 末尾

double LongMemory::simpleTokenSimilarity(const std::string& a, const std::string& b) {
    if (a.empty() || b.empty()) return 0.0;
    std::vector<std::string> tokensA, tokensB;
    std::istringstream issA(a), issB(b);
    std::string token;
    while (issA >> token) tokensA.push_back(token);
    while (issB >> token) tokensB.push_back(token);

    int match = 0;
    for (const auto& tA : tokensA) {
        for (const auto& tB : tokensB) {
            if (tA == tB) { match++; break; }
        }
    }
    int total = std::max(tokensA.size(), tokensB.size());
    return total > 0 ? (double)match / total : 0.0;
}

std::vector<std::pair<int64_t, double>> LongMemory::keywordSearchFacts(const std::string& query, int limit) {
    std::vector<std::pair<int64_t, double>> results;
    if (!_db || query.empty() || limit <= 0) return results;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, fact FROM person_facts WHERE status='active' ORDER BY updated_ts DESC LIMIT 100";
    if (sqlite3_prepare_v2((sqlite3*)_db, sql, -1, &stmt, nullptr) != SQLITE_OK) return results;

    std::vector<std::pair<int64_t, double>> candidates;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int64_t id = sqlite3_column_int64(stmt, 0);
        const char* fact = (const char*)sqlite3_column_text(stmt, 1);
        if (fact) {
            double score = simpleTokenSimilarity(query, fact);
            if (score > 0.1) candidates.push_back({id, score});
        }
    }
    sqlite3_finalize(stmt);

    std::sort(candidates.begin(), candidates.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    for (int i = 0; i < std::min((int)candidates.size(), limit); i++) {
        results.push_back(candidates[i]);
    }
    return results;
}

std::string LongMemory::hybridQueryFacts(const LongMemoryContext& ctx, int limit) {
    if (!_db || limit <= 0) return "";

    std::ostringstream oss;
    std::map<int64_t, double> scoreMap;

    // 60% 权重：向量相似度（简化为关键词相似度）
    auto keywordResults = keywordSearchFacts(ctx.user_message, limit);
    for (const auto& [id, score] : keywordResults) {
        scoreMap[id] += score * 0.6;
    }

    // 20% 权重：置信度排序
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, fact, category, confidence FROM person_facts "
        "WHERE chat_id=? AND chat_type=? AND user_id=? AND status='active' "
        "ORDER BY confidence DESC LIMIT ?";
    if (sqlite3_prepare_v2((sqlite3*)_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, ctx.chat_id);
        sqlite3_bind_text(stmt, 2, ctx.chat_type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, ctx.user_id);
        sqlite3_bind_int(stmt, 4, limit);

        int rank = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW && rank < limit) {
            int64_t id = sqlite3_column_int64(stmt, 0);
            scoreMap[id] += (1.0 - rank * 0.1) * 0.2;
            rank++;
        }
        sqlite3_finalize(stmt);
    }

    // 20% 权重：时间/会话过滤
    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    int64_t sevenDaysAgo = now - 7 * 86400;

    stmt = nullptr;
    sql = "SELECT id, fact, category, confidence FROM person_facts "
        "WHERE chat_id=? AND chat_type=? AND user_id=? AND status='active' AND updated_ts > ? "
        "ORDER BY updated_ts DESC LIMIT ?";
    if (sqlite3_prepare_v2((sqlite3*)_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, ctx.chat_id);
        sqlite3_bind_text(stmt, 2, ctx.chat_type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, ctx.user_id);
        sqlite3_bind_int64(stmt, 4, sevenDaysAgo);
        sqlite3_bind_int(stmt, 5, limit);

        int rank = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW && rank < limit) {
            int64_t id = sqlite3_column_int64(stmt, 0);
            scoreMap[id] += (1.0 - rank * 0.1) * 0.2;
            rank++;
        }
        sqlite3_finalize(stmt);
    }

    // 合并结果并排序
    std::vector<std::pair<int64_t, double>> merged(scoreMap.begin(), scoreMap.end());
    std::sort(merged.begin(), merged.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    // 获取事实详情
    stmt = nullptr;
    sql = "SELECT fact, category, confidence FROM person_facts WHERE id=? LIMIT 1";
    for (int i = 0; i < std::min((int)merged.size(), limit); i++) {
        if (sqlite3_prepare_v2((sqlite3*)_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, merged[i].first);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                const char* fact = (const char*)sqlite3_column_text(stmt, 0);
                const char* category = (const char*)sqlite3_column_text(stmt, 1);
                double conf = sqlite3_column_double(stmt, 2);
                oss << "- " << (category ? category : "fact") << ": "
                    << (fact ? fact : "") << " (相关度:" << merged[i].second << ")\n";
            }
            sqlite3_finalize(stmt);
        }
    }

    return oss.str();
}

void LongMemory::insertRelationEdge(int64_t userId1, int64_t userId2, const std::string& relation) {
    if (!_db || userId1 <= 0 || userId2 <= 0 || relation.empty()) return;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT INTO memory_edges(from_type,from_id,relation,to_type,to_id,weight) "
        "VALUES('user',?,?,?,1) ON CONFLICT DO NOTHING";
    if (sqlite3_prepare_v2((sqlite3*)_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, userId1);
        sqlite3_bind_text(stmt, 2, relation.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, userId2);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

std::string LongMemory::queryGraphRelations(int64_t userId) {
    std::ostringstream oss;
    if (!_db || userId <= 0) return "";

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT relation, to_id FROM memory_edges WHERE from_id=? ORDER BY weight DESC LIMIT 5";
    if (sqlite3_prepare_v2((sqlite3*)_db, sql, -1, &stmt, nullptr) != SQLITE_OK) return "";

    sqlite3_bind_int64(stmt, 1, userId);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* rel = (const char*)sqlite3_column_text(stmt, 0);
        int64_t toId = sqlite3_column_int64(stmt, 1);
        oss << "- " << (rel ? rel : "") << " 用户 " << toId << "\n";
    }
    sqlite3_finalize(stmt);
    return oss.str();
}
