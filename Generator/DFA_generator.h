#pragma once

#include"common.h"
#include<string>
#include<vector>
#include<unordered_map>

class DFA_generator
{
public:
	DFA_generator();
	DFA_generator(const DFA_generator&) = delete;
	DFA_generator(DFA_generator&&) = delete;

	int add_keyword(const std::string& str);
private:
	std::vector<std::string> vec_keywords;
	std::unordered_map<string_hash_type, std::vector<size_t>> umap_hashnum_to_index;	//��vector�洢����Ϊhash������������������ַ�����Ӧͬһ��hashֵ
};