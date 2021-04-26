#pragma once
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Common/multimap_node_manager.h"
#include "Common/unordered_struct_manager.h"

#ifndef GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_
#define GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_

namespace frontend::generator::dfagenerator::nfagenerator {
class NfaGenerator {
 public:
  struct NfaNode;
  using NodeId = common::MultimapNodeManager<NfaNode>::NodeId;
  using NodeGather = size_t;
  using PriorityTag = size_t;
  //ǰ�벿��Ϊtag��ţ���벿��Ϊ���ȼ�������Խ�����ȼ�Խ��
  using TailNodeData = std::pair<size_t, PriorityTag>;

  friend bool MergeNfaNodesWithGenerator(NfaNode& node_dst, NfaNode& node_src,
                                         NfaGenerator& generator);

  struct NfaNode {
    NfaNode() {}
    NfaNode(const NfaNode& node)
        : nodes_forward(node.nodes_forward),
          conditionless_transfer_nodes_handler(
              node.conditionless_transfer_nodes_handler) {}
    NfaNode(NfaNode&& node)
        : nodes_forward(std::move(node.nodes_forward)),
          conditionless_transfer_nodes_handler(
              std::move(node.conditionless_transfer_nodes_handler)) {}

    NodeId GetForwardNodesHandler(char c_transfer);
    const std::unordered_set<NodeId>& GetUnconditionTransferNodesHandler();

    //�������ת�ƽڵ㣬�����Ѵ��ڽڵ�᷵��false
    bool AddConditionTransfer(char c_transfer, NodeId node_handler);
    //���������ת�ƽڵ�
    bool AddNoconditionTransfer(NodeId node_handler);
    //�Ƴ�һ��ת�������ڵ㣬������ִ֤�к󲻴��ڽڵ㣨����ԭ���Ƿ����)
    bool RemoveConditionalTransfer(char c_treasfer);
    //ͬ�ϣ��Ƴ�һ���������ڵ㣬����-1�����������
    bool RemoveConditionlessTransfer(NodeId node_handler);

    bool MergeNodesWithManager(NfaNode& node_src);
    //��¼ת��������ǰ��ڵ㣬һ�������������Ӧһ���ڵ�
    std::unordered_map<char, NodeGather> nodes_forward;
    //�洢������ת�ƽڵ�
    std::unordered_set<NodeId> conditionless_transfer_nodes_handler;
  };

  NfaGenerator() : head_node_handler_(-1) {}
  NfaGenerator(const NfaGenerator&) = delete;
  NfaGenerator(NfaGenerator&&) = delete;

  NodeId GetHeadNodeId() { return head_node_handler_; }

  const TailNodeData GetTailTag(NfaNode* pointer);
  const TailNodeData GetTailTag(NodeId handler);
  NfaNode* GetNode(NodeId handler);
  //����������ӵ�����NFA�У��������ɵ��Զ�����ͷ����β�ڵ㣬�Զ������β�ķ�Χ���Ʒ���
  std::pair<NodeId, NodeId> RegexConstruct(
      std::istream& in, const TailNodeData& tag, bool add_to_NFA_head = true,
      bool return_when_right_bracket = false);
  //���һ�����ַ������ɵ�NFA���Զ������β�ķ�Χ���Ʒ���
  std::pair<NodeId, NodeId> WordConstruct(const std::string& str,
                                          const TailNodeData& tag);
  //�ϲ��Ż������ͽڵ����Խ����Ӽ����취���ϴ�С��ֱ��ʹ��NFAҲ���Խ��ͳɱ�
  void MergeOptimization();

  std::pair<std::unordered_set<NodeId>, TailNodeData> Closure(NodeId handler);
  //����goto��Ľڵ�ıհ�
  std::pair<std::unordered_set<NodeId>, TailNodeData> Goto(NodeId handler_src,
                                                           char c_transform);

  //�������NFA
  void Clear();

  //���л�������
  template <class Archive>
  void Serialize(Archive& ar, const unsigned int version = 0);

  //��β�ڵ���
  const static TailNodeData NotTailNodeTag;

 private:
  bool RemoveTailNode(NfaNode* pointer);
  bool AddTailNode(NfaNode* pointer, const TailNodeData& tag);
  bool AddTailNode(NodeId handler, const TailNodeData& tag) {
    return AddTailNode(GetNode(handler), tag);
  }
  //���ɿ�ѡ�ַ����У����ȡ]���*,+,?���޶���
  std::pair<NodeId, NodeId> CreateSwitchTree(std::istream& in);

  //����NFA��ͷ���
  NodeId head_node_handler_;
  //��set�����洢����β�ڵ�Ͷ�Ӧ���ʵ�tag
  std::unordered_map<NfaNode*, TailNodeData> tail_nodes_;
  common::MultimapNodeManager<NfaNode> node_manager_;
};

bool MergeNfaNodesWithGenerator(NfaGenerator::NfaNode& node_dst,
                                NfaGenerator::NfaNode& node_src,
                                NfaGenerator& generator);
const NfaGenerator::TailNodeData NfaGenerator::NotTailNodeTag =
    NfaGenerator::TailNodeData(-1, -1);
}  // namespace frontend::generator::dfagenerator::nfagenerator
#endif  // !GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_
