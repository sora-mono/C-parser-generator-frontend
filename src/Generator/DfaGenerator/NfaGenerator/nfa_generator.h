#ifndef GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_
#define GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Common/common.h"
#include "Common/id_wrapper.h"
#include "Common/multimap_object_manager.h"
#include "Common/unordered_struct_manager.h"
#include "Generator/export_types.h"

namespace frontend::generator::dfa_generator::nfa_generator {
class NfaGenerator {
 public:
  class NfaNode;

  // NfaNode�ڹ������е�ID
  using NfaNodeId = frontend::common::MultimapObjectManager<NfaNode>::ObjectId;

  // β�ڵ����ݣ�����Ϊ�õ���������������
  // ǰ�벿��Ϊ�û��������ݣ���벿��Ϊ�������ȼ�������Խ�����ȼ�Խ��
  using TailNodeData = std::pair<WordAttachedData, WordPriority>;

  class NfaNode {
   public:
    NfaNode() {}
    NfaNode(const NfaNode& node)
        : nodes_forward_(node.nodes_forward_),
          conditionless_transfer_nodes_id(
              node.conditionless_transfer_nodes_id) {}
    NfaNode(NfaNode&& node)
        : nodes_forward_(std::move(node.nodes_forward_)),
          conditionless_transfer_nodes_id(
              std::move(node.conditionless_transfer_nodes_id)) {}

    // ����ת��������ȡת�Ƶ��Ľڵ�ID
    // �����������Ӧ��ת�ƽ���򷵻�NfaNodeId::InvalidId()
    NfaNodeId GetForwardNodesId(char c_transfer);
    const std::unordered_set<NfaNodeId>& GetUnconditionTransferNodesIds()
        const {
      return conditionless_transfer_nodes_id;
    }
    const std::unordered_map<char, NfaNodeId>& GetConditionalTransfers() const {
      return nodes_forward_;
    }
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

   private:
    friend class NfaGenerator;

    std::unordered_set<NfaNodeId>& GetUnconditionTransferNodesIds() {
      return conditionless_transfer_nodes_id;
    }
    std::unordered_map<char, NfaNodeId>& GetConditionalTransfers() {
      return nodes_forward_;
    }
    // ��¼ת��������ת�Ƶ��Ľڵ㣬һ�������������Ӧһ���ڵ�
    std::unordered_map<char, NfaNodeId> nodes_forward_;
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
  // ����������ӵ�����NFA�У��������ɵ��Զ�����ͷ����β�ڵ�
  // ��Ϊ�ⲿ�ӿ�ʹ��ʱֻ����дǰ�����������������ʹ��Ĭ��ֵ
  // regexΪ������ʽ�ַ�����next_character_indexΪ��һ����ȡ���ַ���λ��
  // add_to_nfa_head�������ַ������Ƿ���ӵ�NFAͷ�����
  // ��ȡ�������ʱ������next_character_index���'('ƥ���')'ʱ����
  // ������У�������next_character_index�����ַ���β��ʱ���أ�
  // ����ʱ���Զ������β�ķ�Χ���Ʒ��ţ�?��+��*�ȣ�
  // �������մ��������һ���ڵ㣨ͷ����β�ڵ���ͬ��
  std::pair<NfaNodeId, NfaNodeId> RegexConstruct(
      const TailNodeData& tail_node_data, const std::string& raw_regex_string,
      size_t&& next_character_index = 0, const bool add_to_nfa_head = true);
  // ���һ�����ַ������ɵ�NFA���Զ������β�ķ�Χ���Ʒ���
  std::pair<NfaNodeId, NfaNodeId> WordConstruct(
      const std::string& str, TailNodeData&& word_attached_data);
  // �ϲ��Ż������ͽڵ����ͷ�֧·���Խ����Ӽ����취���ϴ�С
  // ֱ��ʹ��NFAҲ���Խ��ͳɱ�
  void MergeOptimization();

  // ��ȡ����NFA�ڵ�����е�Ч�ڵ�ID����������
  // �������е�Ч�ڵ�ļ��Ϻ���Щ��Ч�ڵ����ĵ��ʸ������ݣ�������Ч��
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
  // �����������ɿ�ѡ�ַ����У������ȡ]���*,+,?���޶���
  // next_character_indexӦָ��'['�Ҳ��һ���ַ�
  // ����raw_regex_string == "[a-zA-Z_]" next_character_index == 1
  // ���صĵ�һ������Ϊͷ���ID���ڶ�������Ϊβ�ڵ�ID
  std::pair<NfaNodeId, NfaNodeId> CreateSwitchTree(
      const std::string& raw_regex_string, size_t* next_character_index);
  // ��node_src�ϲ���node_dst��
  // �����Ƿ�ϲ��ɹ�
  static bool MergeNfaNodes(NfaGenerator::NfaNode& node_dst,
                            NfaGenerator::NfaNode& node_src,
                            NfaGenerator& nfa_generator);

  // ����NFA��ͷ���
  NfaNodeId head_node_id_;
  // ��set�����洢����β�ڵ�Ͷ�Ӧ���ʵ���Ϣ
  std::unordered_map<const NfaNode*, TailNodeData> tail_nodes_;
  frontend::common::MultimapObjectManager<NfaNode> node_manager_;
};

// Ϊ��SavedData���Բ�������ʵ�ʸýṹ��û���߼�˳��
// ʹ��������ṹ����ע�͵�����
inline bool operator<(const WordAttachedData& left,
                      const WordAttachedData& right) {
  return left.production_node_id < right.production_node_id;
}
inline bool operator>(const WordAttachedData& left,
                      const WordAttachedData& right) {
  return left.production_node_id > right.production_node_id;
}

}  // namespace frontend::generator::dfa_generator::nfa_generator

#endif  // !GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_
