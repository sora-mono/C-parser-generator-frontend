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
	using tail_node_tag = std::pair<size_t, size_t>;	//ǰ�벿��Ϊtag��ţ���벿��Ϊ���ȼ�������Խ�����ȼ�Խ��
	friend bool merge(NFA_node& node_dst, NFA_node& node_src, NFA_generator& generator);

	struct NFA_node
	{
		NFA_node() {}
		NFA_node(const NFA_node& node) :
			m_umap_nodes_forward(node.m_umap_nodes_forward),
			m_uset_nocondition_transfer_nodes_handler(node.m_uset_nocondition_transfer_nodes_handler) {}
		NFA_node(NFA_node&& node) :
			m_umap_nodes_forward(std::move(node.m_umap_nodes_forward)),
			m_uset_nocondition_transfer_nodes_handler(std::move(node.m_uset_nocondition_transfer_nodes_handler)) {}

		node_handler_t get_forward_nodes_handler(char c_transfer);
		const std::unordered_set<node_handler_t>& get_uncondition_transfer_nodes_handler();
		bool add_condition_transfer(char c_transfer, node_handler_t node_handler);		//�������ת�ƽڵ㣬�����Ѵ��ڽڵ�᷵��false
		bool add_nocondition_transfer(node_handler_t node_handler);			//���������ת�ƽڵ�
		bool remove_condition_transfer(char c_treasfer);								//�Ƴ�һ��ת�������ڵ㣬������ִ֤�к󲻴��ڽڵ㣨����ԭ���Ƿ����)
		bool remove_nocondition_transfer(node_handler_t node_handler);		//ͬ�ϣ��Ƴ�һ���������ڵ㣬����-1�����������

		bool merge(NFA_node& node_src);

		std::unordered_map<char, node_gather_t>m_umap_nodes_forward;	//��¼ת��������ǰ��ڵ㣬һ�������������Ӧһ���ڵ�
		std::unordered_set<node_handler_t>m_uset_nocondition_transfer_nodes_handler;	//�洢������ת�ƽڵ�
	};

	NFA_generator();
	NFA_generator(const NFA_generator&) = delete;
	NFA_generator(NFA_generator&&) = delete;

	const tail_node_tag get_tail_tag(NFA_node* pointer);
	const tail_node_tag get_tail_tag(node_handler_t handler);
	NFA_node* get_node(node_handler_t handler);
	//����������ӵ�����NFA�У��������ɵ��Զ�����ͷ����β�ڵ㣬�Զ������β�ķ�Χ���Ʒ���
	std::pair<node_handler_t, node_handler_t> regex_construct(std::istream& in, const tail_node_tag& tag, bool add_to_NFA_head, bool return_when_right_bracket);
	std::pair<node_handler_t, node_handler_t> word_construct(const std::string& str, const tail_node_tag& tag);	//���һ�����ַ������ɵ�NFA���Զ������β�ķ�Χ���Ʒ���
	void merge_optimization();		//�ϲ��Ż������ͽڵ����Խ����Ӽ����취���ϴ�С��ֱ��ʹ��NFAҲ���Խ��ͳɱ�

	std::pair<std::unordered_set<node_handler_t>, tail_node_tag>
		closure(node_handler_t handler);
	std::pair<std::unordered_set<node_handler_t>, tail_node_tag>
		GOTO(node_handler_t handler_src, char c_transform);	//����goto��Ľڵ�ıհ�

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version = 0);		//���л�������

private:
	bool remove_tail_node(NFA_node* pointer);
	bool add_tail_node(NFA_node* pointer, const tail_node_tag& tag);
	bool add_tail_node(node_handler_t handler, const tail_node_tag& tag) { return add_tail_node(get_node(handler), tag); }

	std::pair<node_handler_t, node_handler_t> create_switch_tree(std::istream& in);	//���ɿ�ѡ�ַ����У����ȡ]���*,+,?���޶���

	node_handler_t head_node_handler;		//����NFA��ͷ���
	std::unordered_map<NFA_node*, tail_node_tag> m_umap_tail_nodes;	//��set�����洢����β�ڵ�Ͷ�Ӧ���ʵ�tag
	multimap_node_manager<NFA_node>m_node_manager;
};

bool merge(NFA_generator::NFA_node& node_dst, NFA_generator::NFA_node& node_src, NFA_generator& generator);