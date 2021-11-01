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

class DfaGenerator {
  struct IntermediateDfaNode;

 public:
  // β�ڵ�����
  using TailNodeData = NfaGenerator::TailNodeData;
  // ���������ʺ��浥�ʷ��صĸ�������
  using WordAttachedData = nfa_generator::WordAttachedData;
  // β�ڵ����ȼ�������Խ�����ȼ�Խ�ߣ����б��ʽ�嶼�иò���
  // ��������ڵ�����ȼ����岻ͬ
  // �����ȼ������ڶ������ͬʱƥ��ɹ�ʱѡ���ĸ�����ó��ʺ͸�������
  using WordPriority = nfa_generator::WordPriority;
  // Nfa�ڵ�ID
  using NfaNodeId = NfaGenerator::NfaNodeId;
  // �Ӽ����취ʹ�õļ��ϵ�����
  using SetType = std::unordered_set<NfaNodeId>;
  // �����ϵĽṹ
  using SetManagerType = frontend::common::UnorderedStructManager<SetType>;
  // �Ӽ����취�õ����Ӽ���ID
  using SetId = SetManagerType::ObjectId;
  // �м�ڵ�ID
  using IntermediateNodeId = ObjectManager<IntermediateDfaNode>::ObjectId;

  DfaGenerator() = default;
  DfaGenerator(const DfaGenerator&) = delete;
  DfaGenerator(DfaGenerator&&) = delete;

  // ��ʼ��
  void DfaInit();

  // ��ӵ���
  bool AddWord(const std::string& word, WordAttachedData&& word_attached_data,
               WordPriority priority_tag);
  // �������
  bool AddRegexpression(const std::string& regex_str,
                        WordAttachedData&& regex_attached_data,
                        WordPriority regex_priority);
  // ���������ļ�βʱ���ص�����
  void SetEndOfFileSavedData(WordAttachedData&& saved_data) {
    file_end_saved_data_ = std::move(saved_data);
  }
  // ��ȡ�����ļ�βʱ���ص�����
  const WordAttachedData& GetEndOfFileSavedData() {
    return file_end_saved_data_;
  }
  // ����DFA
  bool DfaConstruct();
  // ������С��DFA
  bool DfaMinimize();
  // ��������
  void SaveConfig() const;

 private:
  friend class boost::serialization::access;

  // ������ϣ�м�ڵ�ID�͵������ݵĽ����Ľṹ������SubDataMinimize
  struct PairOfIntermediateNodeIdAndWordAttachedDataHasher {
    size_t operator()(const std::pair<IntermediateNodeId, WordAttachedData>&
                          data_to_hash) const {
      return data_to_hash.first * data_to_hash.second.production_node_id;
    }
  };

  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    ar << dfa_config_;
    ar << root_transform_array_id_;
    ar << file_end_saved_data_;
  }
  BOOST_SERIALIZATION_SPLIT_MEMBER()

  struct IntermediateDfaNode {
    template <class AttachedData>
    IntermediateDfaNode(SetId handler = SetId::InvalidId(),
                        AttachedData&& word_attached_data_ = WordAttachedData())
        : word_attached_data(std::forward<AttachedData>(word_attached_data_)),
          set_handler(handler) {
      SetAllUntransable();
    }

    // ������������������ת��
    void SetAllUntransable() {
      forward_nodes.fill(IntermediateNodeId::InvalidId());
    }
    SetId GetSetHandler() { return set_handler; }
    void SetWordAttachedData(const WordAttachedData& data) {
      word_attached_data = data;
    }

    // �ýڵ��ת���������洢IntermediateDfaNode�ڵ���
    // ����ֱ��ʹ��CHAR_MIN~CHAR_MAX����ֵ����
    TransformArrayManager<IntermediateNodeId> forward_nodes;
    WordAttachedData word_attached_data;
    // �ýڵ��Ӧ���Ӽ��հ�
    SetId set_handler;
  };

  IntermediateDfaNode& GetIntermediateNode(IntermediateNodeId handler) {
    return node_manager_intermediate_node_.GetObject(handler);
  }
  SetType& GetSetObject(SetId production_node_id) {
    return node_manager_set_.GetObject(production_node_id);
  }
  // ����ת�ƺ������Ӽ����취�ã���
  // ����ֵǰ�벿��Ϊ��Ӧ�м�ڵ�������벿�ֱ�ʾ�Ƿ��´����˼���
  // ����޷�ת���򷵻�std::make_pair(IntermediateNodeId::InvalidId(),false)
  std::pair<IntermediateNodeId, bool> SetGoto(SetId set_src, char c_transform);
  // DFAת�ƺ�������С��DFA�ã�
  IntermediateNodeId IntermediateGoto(IntermediateNodeId dfa_src,
                                      char c_transform);
  // ����DFA�м�ڵ�����ת��
  bool SetIntermediateNodeTransform(IntermediateNodeId node_intermediate_src,
                                    char c_transform,
                                    IntermediateNodeId node_intermediate_dst);
  // ��������Ѵ����򷵻�true���������������벢����false
  // ���صĵ�һ������Ϊ��Ӧ�м�ڵ���
  // ���صĵڶ�������Ϊ�����Ƿ��Ѵ���
  template <class IntermediateNodeSet>
  std::pair<IntermediateNodeId, bool> InOrInsert(
      IntermediateNodeSet&& uset,
      WordAttachedData&& word_attached_data = WordAttachedData());

  // ��handlers�����еľ������С��������CHAR_MINת��������ʼ����ֱ��CHAR_MAX
  // �ڶ���������ʾ��ʼ����
  void SubDfaMinimize(std::list<IntermediateNodeId>&& handlers,
                      char c_transform = CHAR_MIN);

  // DFA���ã�д���ļ�
  DfaConfigType dfa_config_;
  // ͷ�����ţ�д���ļ�
  TransformArrayId root_transform_array_id_;
  // �����ļ�βʱ���ص����ݣ�д���ļ�
  WordAttachedData file_end_saved_data_;

  NfaGenerator nfa_generator_;
  // �м�ڵ�ͷ�����
  IntermediateNodeId head_node_intermediate_;
  // ������Ч�ڵ���
  size_t transform_array_size = 0;

  // �洢DFA�м�ڵ㵽���ձ�ŵ�ӳ��
  std::unordered_map<IntermediateNodeId, TransformArrayId>
      intermediate_node_to_final_node_;
  // �洢�м�ڵ�
  ObjectManager<IntermediateDfaNode> node_manager_intermediate_node_;
  // �Ӽ����취ʹ�õļ��ϵĹ�����
  SetManagerType node_manager_set_;
  // �洢���ϵĹ�ϣ���ڵ�����ӳ��
  std::unordered_map<SetId, IntermediateNodeId> setid_to_intermediate_nodeid_;
};

// ��������Ѵ����򷵻�true���������������벢����false
// ���صĵ�һ������Ϊ��Ӧ�м�ڵ���
// ���صĵڶ�������Ϊ�����Ƿ��Ѵ���

template <class IntermediateNodeSet>
inline std::pair<DfaGenerator::IntermediateNodeId, bool>
DfaGenerator::InOrInsert(IntermediateNodeSet&& uset,
                         WordAttachedData&& word_attached_data) {
  auto [setid, inserted] =
      node_manager_set_.AddObject(std::forward<IntermediateNodeSet>(uset));
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
#endif  // !GENERATOR_DFAGENERATOR_DFAGENERATOR_H_
