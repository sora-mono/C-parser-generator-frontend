#ifndef GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_
#define GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_

#include <any>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>

#include "Common/common.h"
#include "Common/id_wrapper.h"
#include "Common/multimap_object_manager.h"
#include "Common/unordered_struct_manager.h"

namespace frontend::generator::dfa_generator::nfa_generator {
class NfaGenerator {
 public:
  struct NfaNode;
  // �Զ������͵ķַ���ǩ
  enum class WrapperLabel { kTailNodePriority, kTailNodeId };
  // NfaNode�ڹ������е�ID
  using NfaNodeId = frontend::common::MultimapObjectManager<NfaNode>::ObjectId;
  // �������ȼ�
  using WordPriority =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kTailNodePriority>;
  // �������ݣ��ڼ�⵽��Ӧ����ʱ���أ�
  struct WordAttachedData {
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& production_node_id;
      ar& node_type;
      ar& process_function_class_id_;
#ifdef USE_AMBIGUOUS_GRAMMAR
      ar& associate_type;
      ar& operator_priority;
#endif  // USE_AMBIGUOUS_GRAMMAR
    }

    bool operator==(const WordAttachedData& saved_data) const {
      return (
          production_node_id == saved_data.production_node_id &&
          node_type == saved_data.node_type &&
          process_function_class_id_ == saved_data.process_function_class_id_
#ifdef USE_AMBIGUOUS_GRAMMAR
          && associate_type == saved_data.associate_type &&
          operator_priority == saved_data.operator_priority
#endif  // USE_AMBIGUOUS_GRAMMAR
      );
    }
    bool operator!=(const WordAttachedData& saved_data) const {
      return !operator==(saved_data);
    }
    // ����ʽ�ڵ�ID��ǰ�������޷�����Ƕ���࣬�����޷�����Դ����
    // Ӧ��֤ID��Ψһ�ģ���һ��ID��Ӧ��������Ψһ
    size_t production_node_id;
    // �ڵ�����
    frontend::common::ProductionNodeType node_type;
    // ��װ�û�����ĺ�����Ķ����ID
    size_t process_function_class_id_;

    //����������Զ������ķ��Ĳ�������Ч
#ifdef USE_AMBIGUOUS_GRAMMAR
    // �����
    frontend::common::OperatorAssociatityType associate_type;
    // ��������ȼ�
    size_t operator_priority;
#endif  // USE_AMBIGUOUS_GRAMMAR
  };
  // ǰ�벿��Ϊ�û��������ݣ���벿��Ϊ���ȼ�������Խ�����ȼ�Խ��
  using TailNodeData = std::pair<WordAttachedData, WordPriority>;

  // �ϲ�����NFA�ڵ�
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

    // ��������ת�ƽڵ�
    void SetConditionTransfer(char c_transfer, NfaNodeId node_id);
    // ���������ת�ƽڵ�
    void AddNoconditionTransfer(NfaNodeId node_id);
    // �Ƴ�һ��ת�������ڵ㣬������ִ֤�к󲻴��ڽڵ㣨����ԭ���Ƿ����)
    void RemoveConditionalTransfer(char c_treasfer);
    // ͬ�ϣ��Ƴ�һ��������ת�ƽڵ㣬������ִ֤�к󲻴��ڽڵ㣨����ԭ���Ƿ����)
    void RemoveConditionlessTransfer(NfaNodeId node_id);
    // �Ƴ�����������ת�ƽڵ�
    void RemoveAllConditionlessTransfer() {
      conditionless_transfer_nodes_id.clear();
    }

    bool MergeNodesWithManager(NfaNode& node_src);
    // ��¼ת��������ת�Ƶ��Ľڵ㣬һ�������������Ӧһ���ڵ�
    std::unordered_map<char, NfaNodeId> nodes_forward;
    // �洢������ת�ƽڵ�
    std::unordered_set<NfaNodeId> conditionless_transfer_nodes_id;
  };

  NfaGenerator() {}
  NfaGenerator(const NfaGenerator&) = delete;
  NfaGenerator(NfaGenerator&&) = delete;

  NfaNodeId GetHeadNfaNodeId() { return head_node_id_; }

  const TailNodeData GetTailNodeData(NfaNode* pointer);
  const TailNodeData GetTailNodeData(NfaNodeId production_node_id);
  NfaNode& GetNfaNode(NfaNodeId production_node_id) {
    return node_manager_.GetObject(production_node_id);
  }
  // ����������ӵ�����NFA�У��������ɵ��Զ�����ͷ����β�ڵ㣬�Զ������β�ķ�Χ���Ʒ���
  std::pair<NfaNodeId, NfaNodeId> RegexConstruct(
      std::istream& in, const TailNodeData& tag, const bool add_to_NFA_head = true,
      const bool return_when_right_bracket = false);
  // ���һ�����ַ������ɵ�NFA���Զ������β�ķ�Χ���Ʒ���
  std::pair<NfaNodeId, NfaNodeId> WordConstruct(const std::string& str,
                                                const TailNodeData& tag);
  // �ϲ��Ż������ͽڵ����Խ����Ӽ����취���ϴ�С��ֱ��ʹ��NFAҲ���Խ��ͳɱ�
  void MergeOptimization();

  std::pair<std::unordered_set<NfaNodeId>, TailNodeData> Closure(
      NfaNodeId production_node_id);
  // ����goto��Ľڵ�ıհ�
  std::pair<std::unordered_set<NfaNodeId>, TailNodeData> Goto(NfaNodeId id_src,
                                                              char c_transform);

  // NFA��ʼ��
  void NfaInit();

  // ��β�ڵ���
  static const TailNodeData NotTailNodeTag;

 private:
  bool RemoveTailNode(NfaNode* pointer);
  bool AddTailNode(NfaNode* pointer, const TailNodeData& tag);
  bool AddTailNode(NfaNodeId production_node_id, const TailNodeData& tag) {
    return AddTailNode(&GetNfaNode(production_node_id), tag);
  }
  // ���ɿ�ѡ�ַ����У����ȡ]���*,+,?���޶���
  std::pair<NfaNodeId, NfaNodeId> CreateSwitchTree(std::istream& in);

  // ����NFA��ͷ���
  NfaNodeId head_node_id_;
  // ��set�����洢����β�ڵ�Ͷ�Ӧ���ʵ���Ϣ
  std::unordered_map<NfaNode*, TailNodeData> tail_nodes_;
  frontend::common::MultimapObjectManager<NfaNode> node_manager_;
};

bool MergeNfaNodesWithGenerator(NfaGenerator::NfaNode& node_dst,
                                NfaGenerator::NfaNode& node_src,
                                NfaGenerator& generator);
// Ϊ��SavedData���Բ�������ʵ�ʸýṹ��û���߼�˳��
// ʹ��������ṹ����ע�͵�����
inline bool operator<(const NfaGenerator::WordAttachedData& left,
                      const NfaGenerator::WordAttachedData& right) {
  return left.production_node_id < right.production_node_id;
}
inline bool operator>(const NfaGenerator::WordAttachedData& left,
                      const NfaGenerator::WordAttachedData& right) {
  return left.production_node_id > right.production_node_id;
}

}  // namespace frontend::generator::dfa_generator::nfa_generator

#endif  // !GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_
