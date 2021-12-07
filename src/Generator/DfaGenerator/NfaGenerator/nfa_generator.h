/// @file nfa_generator.h
/// @brief NFA����������
/// @details
/// NFA Generator�������������ʽת��Ϊ�������ݽṹ
/// ֧��ʹ��char��ȫ���ַ�
/// ֧�ֻ����������ʽ�У����ַ���[]��[]��ʹ��-��()��*��+��?

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
/// @class NfaGenerator nfa_generator.h
/// @details
/// ��NFA����������֧��char������ֵ��ΪNFA��ת���ַ���֧��*��+��?�޶��ظ�����
/// ����������Ƴ���Ϊ����ΪDFAGenerator�����ʹ�ã�����δ��ӽ�����д���ļ�����
class NfaGenerator {
 private:
  /// @brief �洢NFAת�Ʒ����Ľڵ�
  class NfaNode;

 public:
  /// @brief NfaNode�ڹ������е�ID
  using NfaNodeId = frontend::common::MultimapObjectManager<NfaNode>::ObjectId;

  /// @brief β�ڵ����ݣ�����Ϊ�õ���������������
  /// @details ǰ�벿��Ϊ�û��������ݣ���벿��Ϊ�������ȼ�������Խ�����ȼ�Խ��
  using TailNodeData = std::pair<WordAttachedData, WordPriority>;

 private:
  /// @class NfaNode nfa_generator.h
  /// @brief ��ʾ�����Nfa�ڵ�
  /// @details
  /// 1.NFA���������ʽ�洢���ڴ��У�ÿ���ڵ����������Ա���ֱ�洢�Ӹýڵ����������
  /// ת�Ƶ��Ľڵ������ĳ���ַ���ת�Ƶ��Ľڵ㡣
  /// 2.��һ���ڵ����������ת�Ƶ������нڵ㶼��ýڵ�ȼۣ������ǿ��ԴӸýڵ�ֱ����
  /// ����ת�ƻ��Ǽ��������ת�Ƶõ���
  /// 3.������ת��ģʽ������ʾ���ַ���[]ģʽ��������ת��ģʽ������ʾ*��+��?����������
  /// �����Ӳ��֣�������()�޶�����������ʽ����������ֻɨ��һ���ַ�����ʽ��������ʽ
  /// �͹���������
  /// 4.������ģʽ��ʾ*��+��?�ķ�������ͼ��ʾ��
  /// ͼ��=>����������ת�ƣ�->����������ת�ƣ������ڵ�
  /// ������          ������
  /// ��  ��          ��  ��
  /// ��=>��  ��=>��  ��=>��
  /// ��  ��  ��  ��
  /// ������  ������
  ///   *       +       *
  /// 5.������ת��ֻ�洢����ֱ��ת�Ƶ��Ľڵ㣬�������Լ���ƣ�ͬʱ�����޸ģ��������
  /// ���п���������ת�Ƶ��Ľڵ��������*��+��?ʱ��Ҫ����ȫ��ǰ��Ŀ���������ת�Ƶ�Դ
  /// �ڵ�Ľڵ��������ת�Ʊ���������Ҫ׷��������ӹ��̣�������Ӹ��Ӷ�
  /// 6.β�ڵ���Я����Ч�������ݣ�WordAttachedData���Ľڵ㣬��״̬ת�Ƶ��ýڵ�����
  /// ��ֹת�����̣����ȡ����Ч�ĵ��ʣ��Ѿ�ע����ĵ��ʣ���
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

    /// @brief ����Ҫ�������ĸ��ȡת�Ƶ�����һ���ڵ�ID
    /// @param[in] c_transfer ��Ҫ�������ĸ
    /// @return ת�Ƶ�����һ���ڵ�ID
    /// @retval NfaNodeId::InvalidId() ���ýڵ㲻�����������ĸ
    NfaNodeId GetForwardNodeId(char c_transfer);
    /// @brief ��ȡ���п���������ת�Ƶ��Ľڵ�ID
    /// @return ����������ת�Ƶ��Ľڵ�ID�ļ���
    const std::unordered_set<NfaNodeId>& GetUnconditionTransferNodesIds()
        const {
      return conditionless_transfer_nodes_id;
    }
    /// @brief ��ȡ�ýڵ�ȫ��ת����������ת�������¿���ת�Ƶ��Ľڵ�ID
    const std::unordered_map<char, NfaNodeId>& GetConditionalTransfers() const {
      return nodes_forward_;
    }
    /// @brief ��������ת����Ŀ
    /// @param[in] c_transfer ��ת��������Ҫ������ַ���
    /// @param[out] node_id ��ת�ƺ󵽴�Ľڵ�ID
    /// @note �Ḳ��ԭ����ת��������������ڣ�
    void SetConditionTransfer(char c_transfer, NfaNodeId node_id);
    /// @brief ���������ת�ƽڵ�
    /// @param[in] node_id ��������ת�Ƶ��Ľڵ�ID
    /// @note node_id�����Ѿ���ӹ�
    void AddNoconditionTransfer(NfaNodeId node_id);
    /// @brief �Ƴ�һ��ת����Ŀ
    /// @param[in] c_transfer ��Ҫ�Ƴ���ת����Ŀ��ת������
    /// @return �����Ƴ�����Ŀ��Ŀ
    /// @retval 0 ��û����Ŀ���Ƴ�
    /// @retval 1 ���Ƴ���һ��ת����Ŀ
    size_t RemoveConditionalTransfer(char c_transfer);
    /// @brief �Ƴ�һ��������ת�ƽڵ�
    /// @param[in] node_id ��Ҫ�Ƴ���ת�Ƶ��Ľڵ�ID
    /// @return �����Ƴ�����Ŀ��Ŀ
    /// @retval 0 ��û����Ŀ���Ƴ�
    /// @retval 1 ���Ƴ���һ��ת����Ŀ
    size_t RemoveConditionlessTransfer(NfaNodeId node_id);
    /// @brief �Ƴ�����������ת�ƽڵ�
    void RemoveAllConditionlessTransfer() {
      conditionless_transfer_nodes_id.clear();
    }
    /// @brief �ϲ������ڵ�
    /// @param[in] node_src ��Ҫ�ϲ������Ľڵ�
    /// @return ���غϲ��Ƿ�ɹ�
    /// @retval true ���ϲ��ɹ�
    /// @retval false ��node_src == this�ϲ�ʧ��
    /// @details
    /// �ϲ�Ҫ��node_src��������ת����Ŀ������this�в�����
    /// ����this����ͬ������ת����Ŀת�Ƶ��Ľڵ�ID��ͬ
    /// @note ��node_src�ϲ���this��
    bool MergeNodes(NfaNode* node_src);

   private:
    /// @brief ��¼ת��������ת�Ƶ��Ľڵ㣬һ�������������Ӧһ���ڵ�
    std::unordered_map<char, NfaNodeId> nodes_forward_;
    /// @brief �洢������ת�ƽڵ�
    std::unordered_set<NfaNodeId> conditionless_transfer_nodes_id;
  };

 public:
  NfaGenerator() {}
  NfaGenerator(const NfaGenerator&) = delete;
  NfaGenerator(NfaGenerator&&) = delete;

  /// @brief ��ȡNFA�ڵ�ͷ���ID
  /// @return ����NFA�ڵ�ͷ���ID
  NfaNodeId GetHeadNfaNodeId() const { return head_node_id_; }
  /// @brief ����NFA�ڵ�ID��ȡβ�ڵ�����
  /// @param[in] node_id ��Ҫ��ȡβ�ڵ����ݵ�NFA�ڵ�ID
  /// @return ���ػ�ȡ����β�ڵ�����
  /// @retval ��kNotTailNodeTag node_id����β�ڵ�ʱ���ظ�����
  const TailNodeData& GetTailNodeData(NfaNodeId node_id);
  /// @brief ����NFA�ڵ�ID��ȡNFA�ڵ�
  /// @param[in] node_id ��NFA�ڵ�ID
  /// @return ���ػ�ȡ����NFA�ڵ�
  /// @attention node_id�����Ӧ�Ѵ��ڵ�NFA�ڵ�
  NfaNode& GetNfaNode(NfaNodeId node_id) {
    return node_manager_.GetObject(node_id);
  }
  /// @brief ��������
  /// @param[in] tail_node_data ����ȡ���õ���ʱ���ص�����
  /// @param[in] raw_regex_string ����ʾ���ʵ�������ʽ
  /// @param[in,out] next_character_index
  /// ����һ����ȡ��raw_regex_string�еĵ��ʵ�λ��
  /// @param[in] add_to_nfa_head
  /// ��������ɺ��Ƿ�����NFAͷ��㵽���ɵĽṹ��ͷ����������ת��·��
  /// @return �������ɵ�NFA�ṹ����ͷ���ID��β�ڵ�ID
  /// @details
  /// next_character_index�������øú���ʱ�����￪ʼ��ȡraw_regex_string�е��ַ�
  /// add_to_nfa_head�����ڲ�����'('ʱ�ݹ���øú���ʱ��ֹ����������Ϊ��Ч����
  /// ��ӵ������У���һ�ε���Ӧʹ��true��Ĭ��ֵ�����ݹ����ʱӦʹ��false
  /// ���next_character_indexָ����ַ����Ϊ'('�����ȡ����֮ƥ���')'ʱ����
  /// ����ȡ����һ��')'ʱ���أ���Ϊ�ݹ����ʱ���Զ������Ӧ��')'���������ȡ��
  /// raw_regex_stringĩβʱ����
  /// �������')'������ʱ���Զ�����')'��ķ�Χ���Ʒ��ţ�?��+��*�ȣ�
  /// @note ����ʱnext_character_indexָ����һ��Ҫ��ȡ���ַ�
  /// add_to_nfa_head == falseʱtail_node_data�����ṩ�������
  /// @attention ���next_character_index >= raw_regex_string.size()
  /// �򷵻�ֵ�����ֶ�ΪNfaNodeId::InvalidId()
  std::pair<NfaNodeId, NfaNodeId> RegexConstruct(
      TailNodeData&& tail_node_data, const std::string& raw_regex_string,
      size_t&& next_character_index = 0, const bool add_to_nfa_head = true);
  /// @brief ���һ��������
  /// @param[in] str �����ʵ��ַ���
  /// @param[in] word_attached_data
  /// @return �������ɵ�NFA�ṹ����ͷ���ID��β�ڵ�ID
  /// @details
  /// �ú�����RegexConstruct������������str��Ϊ�����ʶ���������ʽ�����
  /// @return �������ɵ�NFA�ṹ����ͷ����β�ڵ�
  std::pair<NfaNodeId, NfaNodeId> WordConstruct(
      const std::string& str, TailNodeData&& word_attached_data);
  /// @brief �ϲ��Ż������ͽڵ�����ת��·���Խ����Ӽ����취���ϴ�С
  /// @details
  /// �ϲ��Ż�����������ɣ�һ����Ϊɾ����ǰ����ڵ����Ч�ڵ��ظ���ת����
  /// ��һ����Ϊ��ǰ����ڵ������������ת�Ƶ���Ч�ڵ��ת����Ŀʱ�ϲ������ڵ�
  /// ֱ��ʹ��NFAҲ���Խ��ͳɱ�
  void MergeOptimization();

  /// @brief ��ȡ����NFA�ڵ�����е�Ч�ڵ�ID����������
  /// @param[in] node_id ��Ҫ��ȡ���е�Ч�ڵ��NFA�ڵ�ID
  /// @return ǰ�벿��Ϊ���е�Ч�ڵ�ļ��ϣ���벿��Ϊ��Щ��Ч�ڵ����ĵ��ʵ�
  /// ��������
  /// @note ����ֵ��벿��Ϊ���п��ܴ���ĵ�����������ȼ��ĵ��ʵ�����
  /// ������е�Ч�ڵ㶼�޷������κε��ʣ���ô��벿�ַ���kNotTailNodeTag
  std::pair<std::unordered_set<NfaNodeId>, TailNodeData> Closure(
      NfaNodeId node_id);
  /// @brief ��ȡ����NFA�ڵ���ת�������¿��Ե���ĵ�Ч�ڵ�
  /// @param[in] id_src ��Դ�ڵ�
  /// @param[in] c_transform ��ת������
  /// @return ǰ�벿��Ϊ���е�Ч�ڵ�ļ��ϣ���벿��Ϊ��Щ��Ч�ڵ����ĵ��ʵ�
  /// ��������
  /// @details
  /// �ú�����ȡת�ƺ�ﵽ��NfaNodeId��Ȼ�󷵻ظ�ID��Closure���
  std::pair<std::unordered_set<NfaNodeId>, TailNodeData> Goto(NfaNodeId id_src,
                                                              char c_transform);
  /// @brief NFA��ʼ��
  /// @note ����ͨ�����øú���������е�NFA����
  void NfaInit();

  /// @brief ��β�ڵ���
  static const TailNodeData kNotTailNodeTag;

 private:
  /// @brief �Ƴ�β�ڵ���Ϣ
  /// @param[in] tail_node_id ��Ҫ�Ƴ�β�ڵ���Ϣ�Ľڵ�ID
  /// @return ����ɾ������Ŀ����
  /// @retval 0 ��δɾ���κ���Ŀ��node_idָ���Ľڵ㲻���ڻ�Ϊβ�ڵ㣩
  /// @retval 1 ��ɾ�������е�һ����Ŀ
  size_t RemoveTailNode(NfaNodeId tail_node_id) {
    return tail_nodes_.erase(tail_node_id);
  }
  /// @brief ����β�ڵ���Ϣ
  /// @tparam TailNodeDataType ��β�ڵ��������ͣ���֧��const
  /// TailNodeData&��TailNodeData&&
  /// @param[in] node_id ��������Ϊβ�ڵ�Ľڵ�ID
  /// @param[in] tail_node_data ��β�ڵ�����
  /// @return �Ƿ����óɹ�
  /// @retval true �����óɹ�
  /// @retval false ������ʧ��
  /// �Ѵ���node_id��β�ڵ��¼����Ҫ�ȵ���RemoveTailNodeȻ���ٵ��øú���
  template <class TailNodeDataType>
  bool SetTailNode(NfaNodeId node_id, TailNodeDataType&& tail_node_data) {
    return tail_nodes_
        .emplace(node_id, std::forward<TailNodeDataType>(tail_node_data))
        .second;
  }
  /// @brief �����������ɿ�ѡ�ַ��ṹ
  /// @param[in] raw_regex_string ����ʾ���ʵ�������ʽ�ַ���
  /// @param[in,out] next_character_index ��ָ����һ����ȡ���ַ�λ�õ��±��ָ��
  /// @return �������ɵ�NFA���ݽṹ��ͷ���ID��β�ڵ�ID
  /// @details
  /// 1.raw_regex_string�ڶ�Ӧ[]���ֱ���Ϊ��Ч����
  ///   ������std::cerr���������Ϣ���˳�
  /// 2.next_character_index����ʱ�����õ�ֵӦΪ'['�Ҳ��һ���ַ����±�
  ///   �ɹ����ú�����õ�ֵΪ']'�Ҳ��һ���ַ����±�
  /// 3.�����ȡ]���*,+,?���޶���
  /// 4.����raw_regex_string == "[a-zA-Z_]" next_character_index == 1
  std::pair<NfaNodeId, NfaNodeId> CreateSwitchTree(
      const std::string& raw_regex_string, size_t* const next_character_index);

  /// @brief NFAͷ���ID
  NfaNodeId head_node_id_;
  /// @brief ����β�ڵ�ID��β�ڵ����ݵ�ӳ��
  std::unordered_map<NfaNodeId, TailNodeData> tail_nodes_;
  /// @brief ����NFA�ڵ�
  frontend::common::MultimapObjectManager<NfaNode> node_manager_;
};

}  // namespace frontend::generator::dfa_generator::nfa_generator

#endif  /// !GENERATOR_DFAGENERATOR_NFAGENERATOR_NFAGENERATOR_H_
