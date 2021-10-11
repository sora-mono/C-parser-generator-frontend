#ifndef GENERATOR_DFAGENERATOR_DFAGENERATOR_H_
#define GENERATOR_DFAGENERATOR_DFAGENERATOR_H_

#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>

#include "Common/common.h"
#include "Common/id_wrapper.h"
#include "Common/unordered_struct_manager.h"
#include "NfaGenerator/nfa_generator.h"

namespace frontend::parser::dfamachine {
class DfaMachine;
}

namespace frontend::generator::dfa_generator {
using frontend::common::ObjectManager;
using frontend::generator::dfa_generator::nfa_generator::NfaGenerator;
namespace common = frontend::common;

class DfaGenerator {
  struct IntermediateDfaNode;
  // ����ת�Ʊ��ã�������DfaGenerator��Ϊ�˱���ʹ��char���±�ʱʹ�ø��±�
  // ����ֱ��ʹ��CHAR_MIN~CHAR_MAX����ֵ����
  template <class BasicObjectType>
  class TransformArrayManager {
   public:
    TransformArrayManager() {}

    BasicObjectType& operator[](char c) {
      return transform_array_[(c + frontend::common::kCharNum) %
                              frontend::common::kCharNum];
    }
    void fill(const BasicObjectType& fill_object) {
      transform_array_.fill(fill_object);
    }

   private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& transform_array_;
    }

    std::array<BasicObjectType, frontend::common::kCharNum> transform_array_;
  };

 public:
  // β�ڵ�����
  using TailNodeData = NfaGenerator::TailNodeData;
  // ���������ʺ��浥�ʷ��صĸ�������
  using WordAttachedData = TailNodeData::first_type;
  // β�ڵ����ȼ�������Խ�����ȼ�Խ�ߣ����б��ʽ�嶼�иò���
  // ��������ڵ�����ȼ����岻ͬ
  // �����ȼ������ڶ������ͬʱƥ��ɹ�ʱѡ���ĸ�����ó��ʺ͸�������
  using WordPriority = TailNodeData::second_type;
  // Nfa�ڵ�ID
  using NfaNodeId = NfaGenerator::NfaNodeId;
  // �Ӽ����취ʹ�õļ��ϵ�����
  using SetType = std::unordered_set<NfaNodeId>;
  // �м�ڵ�ID
  using IntermediateNodeId = ObjectManager<IntermediateDfaNode>::ObjectId;

  using SetManagerType = common::UnorderedStructManager<SetType>;
  // �Ӽ����취�õ����Ӽ���ID
  using SetId = SetManagerType::ObjectId;
  // �ַ���ǩ
  enum class WrapperLabel { kTransformArrayId };
  // ״̬ת�Ʊ�ID
  using TransformArrayId =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kTransformArrayId>;
  // ״̬ת�Ʊ�
  using TransformArray = TransformArrayManager<TransformArrayId>;
  // DFA��������
  using DfaConfigType =
      std::vector<std::pair<TransformArray, WordAttachedData>>;

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
  // ���½���������������
  bool DfaReconstrcut() { return DfaConstruct() && DfaMinimize(); }
  // ��������
  void SaveConfig() const;

 private:
  friend class DfaMachine;
  friend class boost::serialization::access;

  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    ar << dfa_config_;
    ar << root_index_;
    ar << file_end_saved_data_;
  }
  BOOST_SERIALIZATION_SPLIT_MEMBER()

  struct IntermediateDfaNode {
    IntermediateDfaNode(SetId handler = SetId::InvalidId(),
                        TailNodeData data = NfaGenerator::NotTailNodeTag)
        : tail_node_data(data), set_handler(handler) {
      SetAllUntransable();
    }

    // ������������������ת��
    void SetAllUntransable() {
      forward_nodes.fill(IntermediateNodeId::InvalidId());
    }
    SetId GetSetHandler() { return set_handler; }
    void SetTailNodeData(TailNodeData data) { tail_node_data = data; }

    // �ýڵ��ת���������洢IntermediateDfaNode�ڵ���
    // ����ֱ��ʹ��CHAR_MIN~CHAR_MAX����ֵ����
    TransformArrayManager<IntermediateNodeId> forward_nodes;
    TailNodeData tail_node_data;
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
  std::pair<IntermediateNodeId, bool> InOrInsert(
      const SetType& uset,
      TailNodeData tail_node_data = NfaGenerator::NotTailNodeTag);

  // ��handlers�����еľ������С��������c_transformת��������ʼ
  bool DfaMinimize(const std::vector<IntermediateNodeId>& handlers,
                   char c_transform);
  // DfaMinimize���ӹ��̣����������ɵķ����
  // ����ÿһ����Ҫ��������������DfaMinimize()
  // ������ɵĽڵ����Ӿɽڵ�ID���½ڵ�ID��ӳ��
  // ����ӱ����Ľڵ㵽�½ڵ��ӳ��
  // ��ɾ������ĵ�Ч�ڵ㣬������һ��
  template <class IdType>
  bool DfaMinimizeGroupsRecursion(
      const std::map<IdType, std::vector<IntermediateNodeId>>& groups,
      char c_transform);

  // DFA���ã�д���ļ�
  DfaConfigType dfa_config_;
  // ͷ�����ţ�д���ļ�
  TransformArrayId root_index_;
  // �����ļ�βʱ���ص����ݣ�д���ļ�
  WordAttachedData file_end_saved_data_;

  NfaGenerator nfa_generator_;
  // �м�ڵ�ͷ�����
  IntermediateNodeId head_node_intermediate_;
  // ������Ч�ڵ���
  size_t config_node_num = 0;

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

template <class IdType>
inline bool DfaGenerator::DfaMinimizeGroupsRecursion(
    const std::map<IdType, std::vector<IntermediateNodeId>>& groups,
    char c_transform) {
  for (auto& p : groups) {
    const std::vector<IntermediateNodeId>& vec = p.second;
    if (vec.size() == 1) {
      intermediate_node_to_final_node_.insert(
          std::make_pair(vec.front(), config_node_num));
      ++config_node_num;
      continue;
    }
    if (c_transform != CHAR_MAX) {
      DfaMinimize(vec, c_transform + 1);
    } else {
      intermediate_node_to_final_node_.insert(
          std::make_pair(vec[0], config_node_num));
      for (size_t i = 1; i < vec.size(); i++) {
        intermediate_node_to_final_node_.insert(
            std::make_pair(vec[i], config_node_num));
        node_manager_intermediate_node_.RemoveObject(vec[i]);
      }
      ++config_node_num;
    }
  }
  return true;
}

}  // namespace frontend::generator::dfa_generator
#endif  // !GENERATOR_DFAGENERATOR_DFAGENERATOR_H_
