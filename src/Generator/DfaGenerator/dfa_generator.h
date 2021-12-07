/// @file dfa_generator.h
/// @brief DFA����������
/// @details
/// DFA����������ʹ���Ӽ����취��NFA���û����Ϲ���DFA����������ٶ�
/// DFA Generator��������ʱ��ͨ���Ӽ����취�����м�ڵ㣨IntermediateDfaNode����
/// ÿ���м�ڵ��Ӧ�Ӽ����취��Ψһ��һ�����ϣ�Ȼ��ͨ���м�ڵ㹹��DFAת�Ʊ�
#ifndef GENERATOR_DFAGENERATOR_DFAGENERATOR_H_
#define GENERATOR_DFAGENERATOR_DFAGENERATOR_H_

#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>

#include "Common/common.h"
#include "Common/id_wrapper.h"
#include "Common/object_manager.h"
#include "Common/unordered_struct_manager.h"
#include "Generator/export_types.h"
#include "NfaGenerator/nfa_generator.h"

namespace frontend::generator::dfa_generator {
using frontend::common::ObjectManager;
using frontend::generator::dfa_generator::nfa_generator::NfaGenerator;

/// @class DfaGenerator dfa_generator.h
/// @brief DFA����������
class DfaGenerator {
  // @brief ǰ�������м�ڵ��Զ���IntermediateNodeId
  struct IntermediateDfaNode;

 public:
  /// @brief Nfa�ڵ�ID
  using NfaNodeId = NfaGenerator::NfaNodeId;
  /// @brief �Ӽ����취ʹ�õļ��ϵ�����
  using SetType = std::unordered_set<NfaNodeId>;
  /// @class UnorderedSetHasher dfa_generator.h
  /// @brief ��ϣSetType���ϵ��࣬����ʵ����UnorderedStructManager
  struct UnorderedSetHasher {
    size_t operator()(const SetType& set) const {
      size_t result = 1;
      for (const auto& item : set) {
        result *= item;
      }
      return result;
    }
  };
  /// @brief β�ڵ�����
  using TailNodeData = NfaGenerator::TailNodeData;
  /// @brief β�ڵ����ȼ�
  /// @details ����Խ�����ȼ�Խ��
  /// �����ս����ʽ�ĵ��ʶ��иò���
  /// ��������ڵ�����ȼ����岻ͬ�������ȼ������ڶ������ͬʱƥ��ɹ�ʱѡ���ĸ�
  /// ����ó��ʺ͸�������
  /// @note ��ͨ����ʹ�����ȼ�0�������ʹ�����ȼ�1���ؼ���ʹ�����ȼ�2
  using WordPriority = nfa_generator::WordPriority;
  /// @brief ���������ʺ��浥�ʷ��صĸ�������
  using WordAttachedData = nfa_generator::WordAttachedData;
  /// @brief �����Ӽ����취ʹ�õļ��ϵĽṹ
  using SetManagerType =
      frontend::common::UnorderedStructManager<SetType, UnorderedSetHasher>;
  /// @brief �Ӽ����취ʹ�õļ��ϵ�ID
  using SetId = SetManagerType::ObjectId;
  /// @brief DFA�м�ڵ�ID
  using IntermediateNodeId = ObjectManager<IntermediateDfaNode>::ObjectId;

  DfaGenerator() = default;
  DfaGenerator(const DfaGenerator&) = delete;
  DfaGenerator(DfaGenerator&&) = delete;

  /// @brief ��ʼ��
  /// @note �ú��������������������ӵ���
  void DfaInit();

  /// @brief ��Ӵ�����
  /// @param[in] word ������ӵĵ���
  /// @param[in] word_attached_data �����ʵĸ�������
  /// @param[in] priority_tag �����ʵ����ȼ�
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ����ӳɹ�
  /// @retval false �����ʧ��
  /// @note �������ȼ������WordPriority˵��
  /// @attention �ú�����word��Ϊ�����ʶ�����������ʽ�������
  bool AddWord(const std::string& word, WordAttachedData&& word_attached_data,
               WordPriority priority_tag);
  /// @brief ��ӵ��ʵ�����
  /// @param[in] regex_str ������ӵ��ʵ�������ʽ
  /// @param[in] regex_attached_data �����ʵĸ�������
  /// @param[in] regex_priority �����ʵ����ȼ�
  /// @return �����Ƿ���ӳɹ�
  /// @retval true ����ӳɹ�
  /// @retval false �����ʧ��
  /// @note �������ȼ������WordPriority˵��
  bool AddRegexpression(const std::string& regex_str,
                        WordAttachedData&& regex_attached_data,
                        WordPriority regex_priority);
  /// @brief ���õ����ļ�βʱ���ص�����
  /// @param[in] saved_data �������ļ�β���ص�����
  /// @attention ����δ��ȡ���κε����ҵ����ļ�βʱ����
  /// ����Ѿ���ȡ��һЩ�ַ��������ļ�β�򷵻���Щ�ַ���Ӧ�ĵ���
  void SetEndOfFileSavedData(WordAttachedData&& saved_data) {
    file_end_saved_data_ = std::move(saved_data);
  }
  /// @brief ��ȡ�����ļ�βʱ���ص�����
  /// @return ���ص����ļ�βʱ���ص����ݵ�const����
  const WordAttachedData& GetEndOfFileSavedData() const {
    return file_end_saved_data_;
  }
  /// @brief ����DFA����
  /// @note �ú���������DFA���ã������Զ��������õ��ļ�
  bool DfaConstruct();
  /// @brief ����DFA����
  void SaveConfig() const;

 private:
  /// @brief ������Ԫ���������л�����ʳ�Ա
  friend class boost::serialization::access;

  /// @class PairOfIntermediateNodeIdAndWordAttachedDataHasher dfa_generator.h
  /// @brief ��ϣ�м�ڵ�ID�͵������ݵĽ����Ľṹ������SubDataMinimize
  struct PairOfIntermediateNodeIdAndWordAttachedDataHasher {
    size_t operator()(const std::pair<IntermediateNodeId, WordAttachedData>&
                          data_to_hash) const {
      return data_to_hash.first * data_to_hash.second.production_node_id;
    }
  };

  /// @brief ���л����õĺ���
  /// @param[in,out] ar �����л�ʹ�õĵ���
  /// @param[in] version �����л��ļ��汾
  /// @attention �ú���Ӧ��boost����ö����ֶ�����
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    ar << dfa_config_;
    ar << root_transform_array_id_;
    ar << file_end_saved_data_;
  }
  /// �ָ�save��load������DFA����������ֻ��ִ��save����������ִ��load����
  BOOST_SERIALIZATION_SPLIT_MEMBER()

  /// @class IntermediateDfaNode dfa_generator.h
  /// @brief DFA�������������м�ڵ�
  /// @details
  /// �м�ڵ����Ӽ����취�ļ���һһ��Ӧ��ÿ���ڵ�洢һ������ת�Ʊ�
  struct IntermediateDfaNode {
    template <class AttachedData>
    IntermediateDfaNode(SetId handler = SetId::InvalidId(),
                        AttachedData&& word_attached_data_ = WordAttachedData())
        : word_attached_data(std::forward<AttachedData>(word_attached_data_)),
          set_handler(handler) {
      SetAllUntransable();
    }

    /// @brief ���øýڵ����κ��ַ��¾�����ת��
    void SetAllUntransable() {
      forward_nodes.fill(IntermediateNodeId::InvalidId());
    }
    /// @brief ��ȡ�ýڵ��Ӧ���ϵ�ID
    /// @return ���ظýڵ��Ӧ���ϵ�ID
    SetId GetSetHandler() const { return set_handler; }
    /// @brief ���øýڵ��Ӧ���ʵ�����
    /// @param[in] data ����Ӧ��������
    /// @note �ú���������WordAttachedData��const���ú���ֵ����
    template <class AttachedData>
    void SetWordAttachedData(AttachedData&& data) {
      word_attached_data = std::forward<AttachedData>(data);
    }

    /// @brief �ýڵ��ת���������洢IntermediateDfaNode�ڵ���
    /// @note ����ֱ��ʹ��CHAR_MIN~CHAR_MAX����ֵ���ʣ���������±����
    TransformArrayManager<IntermediateNodeId> forward_nodes;
    /// @brief �ýڵ��Ӧ���ʵ�����
    WordAttachedData word_attached_data;
    /// �ýڵ��Ӧ�ļ���ID
    SetId set_handler;
  };

  /// @brief ������С��DFA
  /// @note �ú�����DfaConstruct��������ã��ϲ���ͬת������DFA���ñ�
  bool DfaMinimize();
  /// @brief ��ȡ�м�ڵ�����
  /// @param[in] id ���м�ڵ�ID
  /// @return ���ض�Ӧ���м�ڵ�����
  /// @note id�����Ӧ�Ѵ��ڵ��м�ڵ�
  IntermediateDfaNode& GetIntermediateNode(IntermediateNodeId id) {
    return node_manager_intermediate_node_.GetObject(id);
  }
  /// @brief ��ȡ�м�ڵ�const����
  /// @param[in] id ���м�ڵ�ID
  /// @return ���ض�Ӧ���м�ڵ�const����
  /// @note id�����Ӧ�Ѵ��ڵ��м�ڵ�
  const IntermediateDfaNode& GetIntermediateNode(IntermediateNodeId id) const {
    return node_manager_intermediate_node_.GetObject(id);
  }
  /// @brief ��ȡ�Ӽ����취�ļ��ϵ�����
  /// @param[in] set_id ������ID
  /// @return ���ؼ��ϵ�����
  /// @note set_id�����Ӧ�Ѵ��ڵļ���
  SetType& GetSetObject(SetId set_id) {
    return node_manager_set_.GetObject(set_id);
  }
  /// @brief ��ȡNFA�ڵ㼯���ڸ����ַ��µ�ת�ƽ��
  /// @param[in] set_src ��ת������NFA�ڵ�ID�ļ���
  /// @param[in] c_transform ��ת������
  /// @return ����ֵǰ�벿��Ϊ��Ӧ�м�ڵ�������벿�ֱ�ʾ�Ƿ��´����˼���
  /// @retval std::pair(IntermediateNodeId::InvalidId(), false)
  /// ���ü����ڸ����������޷�ת��
  std::pair<IntermediateNodeId, bool> SetGoto(SetId set_src, char c_transform);
  /// @brief ��ѯDFA�м�ڵ��ڸ��������µ�ת�����
  /// @param[in] dfa_src ��ת������DFA�м�ڵ�
  /// @param[in] c_transform ��ת������
  /// @return ����ת�Ƶ���DFA�м�ڵ�ID
  /// @retval IntermediateNodeId::InvalidId()
  /// ���ýڵ���c_transform�������޷�ת��
  /// @note �ú�������ѯ��SetGoto��������ڻᴴ���¼��Ϻ��м�ڵ�
  IntermediateNodeId IntermediateGoto(IntermediateNodeId dfa_src,
                                      char c_transform) const;
  /// @brief ����DFA�м�ڵ�����ת��
  /// @param[in] node_intermediate_src ��Դ�ڵ�
  /// @param[in] c_transform ��ת������
  /// @param[in] node_intermediate_dst ��Ŀ�Ľڵ�
  /// @return �����Ƿ����óɹ�
  /// @retval true �����óɹ�
  /// @retval false ������ʧ��
  /// @note һ������true
  bool SetIntermediateNodeTransform(IntermediateNodeId node_intermediate_src,
                                    char c_transform,
                                    IntermediateNodeId node_intermediate_dst);
  /// @brief ��ȡ���϶�Ӧ���м�ڵ�ID������������򴴽��µ��м�ڵ�
  /// @param[in] uset ����ȡ��Ӧ�м�ڵ�ļ���
  /// @param[in] word_attached_data ����������м�ڵ�����м�ڵ�ʹ�øõ�������
  /// @return
  /// ���ػ�ȡ�����м�ڵ�ID���Ƿ��²���ü��ϣ��Ƿ��½��м�ڵ㣩
  /// @note IntermediateNodeSet��֧��SetType��const��ֵ���û���ֵ����
  template <class IntermediateNodeSet>
  std::pair<IntermediateNodeId, bool> InOrInsert(
      IntermediateNodeSet&& uset,
      WordAttachedData&& word_attached_data = WordAttachedData());

  /// @brief ���м�ڵ���з���
  /// @param[in] node_ids ��������Ľڵ㼯��
  /// @param[in] c_transform �����ַ���ʹ�õ�ת���ַ�
  /// @details
  /// �ú���ÿ�ε��ö��Ե�ǰ�м�ڵ�����ڵ�ǰת���ַ��µ�ת�ƽ�����з���
  /// ���෽����
  /// ÿ�ַ����õ�>=1���Ӽ��ϣ�����Ӽ����ں��ж���ڵ���c_transform!=CHAR_MIN
  /// ��ʹ��c_transform+1��ÿ�����ж��Ԫ�صļ��ϵ��øú���������һ�ַ���
  /// ����ÿ���Ӽ���ӳ��Ϊһ��DFAת�Ʊ���Ŀ��������������нڵ��Ч
  /// ���÷�ʽ��
  /// ���ε���ʱc_transform�����һ����ʼ�жϵ��ַ�
  /// ÿ�ֵ��ú������Ҫ������һ�ַ������Զ����ã������ֶ�����
  /// @note ÿһ��ת�ƽ������ͬ���м�ڵ�Ϊ��Ч�ڵ㣬���Ժϲ�
  /// �ú�������дintermediate_node_to_final_node_��transform_array_size_
  /// @attention node_ids����Ԫ�ز������ظ�
  void IntermediateNodeClassify(std::list<IntermediateNodeId>&& node_ids,
                                char c_transform = CHAR_MIN);

  /// @brief DFA����
  /// @note д�������ļ�
  DfaConfigType dfa_config_;
  /// @brief DFAת�Ʊ��ʼ��Ŀ���
  /// @note д�������ļ�
  TransformArrayId root_transform_array_id_;
  /// @brief �����ļ�βʱ���ص�����
  /// @note д�������ļ�
  WordAttachedData file_end_saved_data_;

  /// @brief NFA����������
  NfaGenerator nfa_generator_;
  /// @brief �м�ڵ��ʼ�ڵ�ID
  IntermediateNodeId root_intermediate_node_id_;
  /// @brief DFAת�Ʊ���Ŀ��
  size_t transform_array_size_ = 0;

  /// @brief �洢DFA�м�ڵ㵽ת�Ʊ���Ŀ��ӳ��
  std::unordered_map<IntermediateNodeId, TransformArrayId>
      intermediate_node_to_final_node_;
  /// @brief �洢�м�ڵ�
  ObjectManager<IntermediateDfaNode> node_manager_intermediate_node_;
  /// @brief �����м�ڵ��Ӧ�ļ���
  SetManagerType node_manager_set_;
  /// @brief �洢����ID���м�ڵ�ID��ӳ��
  std::unordered_map<SetId, IntermediateNodeId> setid_to_intermediate_nodeid_;
};

template <class IntermediateNodeSet>
inline std::pair<DfaGenerator::IntermediateNodeId, bool>
DfaGenerator::InOrInsert(IntermediateNodeSet&& uset,
                         WordAttachedData&& word_attached_data) {
  auto [setid, inserted] =
      node_manager_set_.EmplaceObject(std::forward<IntermediateNodeSet>(uset));
  IntermediateNodeId intermediate_node_id = IntermediateNodeId::InvalidId();
  if (inserted) {
    intermediate_node_id = node_manager_intermediate_node_.EmplaceObject(
        setid, std::move(word_attached_data));
    setid_to_intermediate_nodeid_.insert(
        std::make_pair(setid, intermediate_node_id));
  } else {
    auto iter = setid_to_intermediate_nodeid_.find(setid);
    assert(iter != setid_to_intermediate_nodeid_.end());
    intermediate_node_id = iter->second;
  }
  return std::make_pair(intermediate_node_id, inserted);
}

}  // namespace frontend::generator::dfa_generator
#endif  /// !GENERATOR_DFAGENERATOR_DFAGENERATOR_H_
