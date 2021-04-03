#pragma once

#include"multimap_node_manager.h"

#include<iostream>
#include<vector>
#include<unordered_map>
#include<unordered_set>

class NFA_generator
{
public:
	struct NFA_node;
	using node_handler_t = multimap_node_manager<NFA_node>::node_handler_t;
	using node_gather_t = size_t;

	struct NFA_node
	{
		NFA_node(){}
		NFA_node(const NFA_node& node) :
			m_umap_nodes_forward(node.m_umap_nodes_forward),
			m_vec_nocondition_transfer_nodes_index(node.m_vec_nocondition_transfer_nodes_index) {}
		NFA_node(NFA_node&& node) :
			m_umap_nodes_forward(std::move(node.m_umap_nodes_forward)),
			m_vec_nocondition_transfer_nodes_index(std::move(node.m_vec_nocondition_transfer_nodes_index)) {}

		node_handler_t get_forward_nodes_index(char c_transfer);
		const std::vector<node_handler_t>& get_uncondition_transfer_nodes_index();
		bool add_transfer(char c_transfer, node_handler_t node_index);	//���ת�������½ڵ㣬�����Ѵ��ڽڵ�᷵��false
		bool add_nocondition_transfer(node_handler_t node_index);			//���������ת�ƽڵ�
		bool remove_transfer(char c_treasfer);							//�Ƴ�ת��������������ִ֤�к󲻴��ڽڵ㣨����ԭ���Ƿ����)
		bool remove_nocondition_transfer(node_handler_t node_index);		//ͬ�ϣ��Ƴ��������ڵ㣬����-1�����������

		std::unordered_map<char, node_gather_t>m_umap_nodes_forward;	//��¼ת��������ǰ��ڵ㣬һ�������������Ӧһ���ڵ�
		std::vector<node_handler_t>m_vec_nocondition_transfer_nodes_index;	//�洢������ת�ƽڵ�
	};

	NFA_generator();
	NFA_generator(const NFA_generator&) = delete;
	NFA_generator(NFA_generator&&) = delete;

	//����tail������ת�Ƶ�head�ļ�¼����������allow_merge����ܽ�head�ϲ���tail���޸�tailָ��ϲ���ڵ㣬���ڲ�����head�ڵ��������ת��ʱ���ø�λ
	bool add_nocondition_transfer(node_handler_t node_tail_index, node_handler_t& node_head_index, bool allow_merge);
	std::pair<node_handler_t, node_handler_t> regex_construct(std::istream& in, bool add_to_NFA_head, bool return_when_right_bracket);	//����������ӵ�����NFA�У��������ɵ��Զ�����ͷ����β�ڵ�
	std::pair<node_handler_t, node_handler_t> word_construct(const std::string& str);	//���һ�����ַ������ɵ�NFA

private:
	node_handler_t head_node_index;		//����NFA��ͷ���

	auto add_tail_node(node_handler_t index) { m_uset_tail_nodes.insert(index); }
	std::pair<node_handler_t, node_handler_t> create_switch_tree(std::istream& in);	//���ɿ�ѡ�ַ����У����ȡ]���*,+,?���޶���

	std::unordered_set<node_handler_t> m_uset_tail_nodes;	//��set�����洢����β�ڵ�
	multimap_node_manager<NFA_node>m_node_manager;
};