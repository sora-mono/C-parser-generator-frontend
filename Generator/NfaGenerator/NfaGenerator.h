#pragma once

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "MultimapNodeManager.h"

class NfaGenerator {
 public:
  struct NfaNode;
  using NodeHandler = MultimapNodeManager<NfaNode>::NodeHandler;
  using NodeGather = size_t;
  //ǰ�벿��Ϊtag��ţ���벿��Ϊ���ȼ�������Խ�����ȼ�Խ��
  using TailNodeTag = std::pair<size_t, size_t>;
  friend bool MergeNfaNodesWithGenerator(NfaNode& node_dst, NfaNode& node_src,
                                         NfaGenerator& generator);

  struct NfaNode {
    NfaNode() {}
    NfaNode(const NfaNode& node)
        : nodes_forward_(node.nodes_forward_),
          conditionless_transfer_nodes_handler_(
              node.conditionless_transfer_nodes_handler_) {}
    NfaNode(NfaNode&& node)
        : nodes_forward_(std::move(node.nodes_forward_)),
          conditionless_transfer_nodes_handler_(
              std::move(node.conditionless_transfer_nodes_handler_)) {}

    NodeHandler GetForwardNodesHandler(char c_transfer);
    const std::unordered_set<NodeHandler>& GetUnconditionTransferNodesHandler();

    //�������ת�ƽڵ㣬�����Ѵ��ڽڵ�᷵��false
    bool AddConditionTransfer(char c_transfer, NodeHandler node_handler);
    //���������ת�ƽڵ�
    bool AddNoconditionTransfer(NodeHandler node_handler);
    //�Ƴ�һ��ת�������ڵ㣬������ִ֤�к󲻴��ڽڵ㣨����ԭ���Ƿ����)
    bool RemoveConditionalTransfer(char c_treasfer);
    //ͬ�ϣ��Ƴ�һ���������ڵ㣬����-1�����������
    bool RemoveConditionlessTransfer(NodeHandler node_handler);

    bool MergeNodesWithManager(NfaNode& node_src);
    //��¼ת��������ǰ��ڵ㣬һ�������������Ӧһ���ڵ�
    std::unordered_map<char, NodeGather> nodes_forward_;
    //�洢������ת�ƽڵ�
    std::unordered_set<NodeHandler> conditionless_transfer_nodes_handler_;
  };

  NfaGenerator();
  NfaGenerator(const NfaGenerator&) = delete;
  NfaGenerator(NfaGenerator&&) = delete;

  const TailNodeTag get_tail_tag(NfaNode* pointer);
  const TailNodeTag get_tail_tag(NodeHandler handler);
  NfaNode* GetNode(NodeHandler handler);
  //����������ӵ�����NFA�У��������ɵ��Զ�����ͷ����β�ڵ㣬�Զ������β�ķ�Χ���Ʒ���
  std::pair<NodeHandler, NodeHandler> regex_construct(
      std::istream& in, const TailNodeTag& tag, bool add_to_NFA_head,
      bool return_when_right_bracket);
  //���һ�����ַ������ɵ�NFA���Զ������β�ķ�Χ���Ʒ���
  std::pair<NodeHandler, NodeHandler> word_construct(const std::string& str,
                                                     const TailNodeTag& tag);
  //�ϲ��Ż������ͽڵ����Խ����Ӽ����취���ϴ�С��ֱ��ʹ��NFAҲ���Խ��ͳɱ�
  void merge_optimization();

  std::pair<std::unordered_set<NodeHandler>, TailNodeTag> Closure(
      NodeHandler handler);
  //����goto��Ľڵ�ıհ�
  std::pair<std::unordered_set<NodeHandler>, TailNodeTag> Goto(
      NodeHandler handler_src, char c_transform);
  //���л�������
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version = 0);

 private:
  bool remove_tail_node(NfaNode* pointer);
  bool add_tail_node(NfaNode* pointer, const TailNodeTag& tag);
  bool add_tail_node(NodeHandler handler, const TailNodeTag& tag) {
    return add_tail_node(GetNode(handler), tag);
  }
  //���ɿ�ѡ�ַ����У����ȡ]���*,+,?���޶���
  std::pair<NodeHandler, NodeHandler> create_switch_tree(std::istream& in);

  //����NFA��ͷ���
  NodeHandler head_node_handler_;
  //��set�����洢����β�ڵ�Ͷ�Ӧ���ʵ�tag
  std::unordered_map<NfaNode*, TailNodeTag> tail_nodes_;
  MultimapNodeManager<NfaNode> node_manager_;
};

bool MergeNfaNodesWithGenerator(NfaGenerator::NfaNode& node_dst,
                                NfaGenerator::NfaNode& node_src,
                                NfaGenerator& generator);