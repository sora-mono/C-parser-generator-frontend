#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Common/id_wrapper.h"
#include "Common/multimap_object_manager.h"
#include "Common/unordered_struct_manager.h"

#ifndef GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_
#define GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_

namespace frontend::generator::dfa_generator::nfa_generator {
class NfaGenerator {
 public:
  struct NfaNode;
  //�Զ������͵ķַ���ǩ
  enum class WrapperLabel { kTailNodePriority, kTailNodeId };
  // NfaNode�ڹ������е�ID
  using NfaNodeId = frontend::common::MultimapObjectManager<NfaNode>::ObjectId;
  //�������ȼ�
  using TailNodePriority =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kTailNodePriority>;
  //β�ڵ�ID�����û����룬�ڼ�⵽��Ӧ�ڵ�ʱ���أ�
  using TailNodeId =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kTailNodeId>;
  //ǰ�벿��Ϊtag��ţ���벿��Ϊ���ȼ�������Խ�����ȼ�Խ��
  using TailNodeData = std::pair<TailNodeId, TailNodePriority>;

  //�ϲ�����NFA�ڵ�
  // TODO �˽������Ԫ�����Ƿ��Ҫ
  friend bool MergeNfaNodesWithGenerator(NfaNode& node_dst, NfaNode& node_src,
                                         NfaGenerator& generator);

  // TODO ����struct��Ϊclass
  struct NfaNode {
    NfaNode() {}
    NfaNode(const NfaNode& node)
        : nodes_forward(node.nodes_forward),
          conditionless_transfer_nodes_id(
              node.conditionless_transfer_nodes_id) {}
    NfaNode(NfaNode&& node)
        : nodes_forward(std::move(node.nodes_forward)),
          conditionless_transfer_nodes_id(
              std::move(node.conditionless_transfer_nodes_id)) {}

    NfaNodeId GetForwardNodesId(char c_transfer);
    const std::unordered_set<NfaNodeId>& GetUnconditionTransferNodesId();

    //��������ת�ƽڵ�
    void SetConditionTransfer(char c_transfer, NfaNodeId node_id);
    //���������ת�ƽڵ�
    void AddNoconditionTransfer(NfaNodeId node_id);
    //�Ƴ�һ��ת�������ڵ㣬������ִ֤�к󲻴��ڽڵ㣨����ԭ���Ƿ����)
    void RemoveConditionalTransfer(char c_treasfer);
    //ͬ�ϣ��Ƴ�һ��������ת�ƽڵ㣬������ִ֤�к󲻴��ڽڵ㣨����ԭ���Ƿ����)
    void RemoveConditionlessTransfer(NfaNodeId node_id);
    //�Ƴ�����������ת�ƽڵ�
    void RemoveAllConditionlessTransfer() {
      conditionless_transfer_nodes_id.clear();
    }

    bool MergeNodesWithManager(NfaNode& node_src);
    //��¼ת��������ת�Ƶ��Ľڵ㣬һ�������������Ӧһ���ڵ�
    std::unordered_map<char, NfaNodeId> nodes_forward;
    //�洢������ת�ƽڵ�
    std::unordered_set<NfaNodeId> conditionless_transfer_nodes_id;
  };

  NfaGenerator();
  NfaGenerator(const NfaGenerator&) = delete;
  NfaGenerator(NfaGenerator&&) = delete;

  NfaNodeId GetHeadNfaNodeId() { return head_node_id_; }

  const TailNodeData GetTailTag(NfaNode* pointer);
  const TailNodeData GetTailTag(NfaNodeId id);
  NfaNode& GetNfaNode(NfaNodeId id) {
    return node_manager_.GetObject(id);
  }
  //����������ӵ�����NFA�У��������ɵ��Զ�����ͷ����β�ڵ㣬�Զ������β�ķ�Χ���Ʒ���
  std::pair<NfaNodeId, NfaNodeId> RegexConstruct(
      std::istream& in, const TailNodeData& tag, bool add_to_NFA_head = true,
      bool return_when_right_bracket = false);
  //���һ�����ַ������ɵ�NFA���Զ������β�ķ�Χ���Ʒ���
  std::pair<NfaNodeId, NfaNodeId> WordConstruct(const std::string& str,
                                                const TailNodeData& tag);
  //�ϲ��Ż������ͽڵ����Խ����Ӽ����취���ϴ�С��ֱ��ʹ��NFAҲ���Խ��ͳɱ�
  void MergeOptimization();

  std::pair<std::unordered_set<NfaNodeId>, TailNodeData> Closure(
      NfaNodeId id);
  //����goto��Ľڵ�ıհ�
  std::pair<std::unordered_set<NfaNodeId>, TailNodeData> Goto(
      NfaNodeId id_src, char c_transform);

  //�������NFA
  void Clear();

  //���л�������
  template <class Archive>
  void Serialize(Archive& ar, const unsigned int version = 0);

  //��β�ڵ���
  static const TailNodeData NotTailNodeTag;

 private:
  bool RemoveTailNode(NfaNode* pointer);
  bool AddTailNode(NfaNode* pointer, const TailNodeData& tag);
  bool AddTailNode(NfaNodeId id, const TailNodeData& tag) {
    return AddTailNode(&GetNfaNode(id), tag);
  }
  //���ɿ�ѡ�ַ����У����ȡ]���*,+,?���޶���
  std::pair<NfaNodeId, NfaNodeId> CreateSwitchTree(std::istream& in);

  //����NFA��ͷ���
  NfaNodeId head_node_id_;
  //��set�����洢����β�ڵ�Ͷ�Ӧ���ʵ�tag
  std::unordered_map<NfaNode*, TailNodeData> tail_nodes_;
  common::MultimapObjectManager<NfaNode> node_manager_;
};

bool MergeNfaNodesWithGenerator(NfaGenerator::NfaNode& node_dst,
                                NfaGenerator::NfaNode& node_src,
                                NfaGenerator& generator);

}  // namespace frontend::generator::dfa_generator::nfa_generator
#endif  // !GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_
