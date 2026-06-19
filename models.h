#pragma once
#include"base.h"
#include"deepseek.h"

class Models {
public:
	Deepseek deepseek;

	// 调用 embedding API 获取文本向量
	std::vector<double> getEmbedding(CURL* curl, const std::string& text);
};