#pragma once

#include <array>
#include <map>

#include "Common/hash_functions.h"
#include "Common/id_wrapper.h"
#include "Common/unordered_struct_manager.h"
#include "NfaGenerator/nfa_generator.h"

#ifndef GENERATOR_DFAGENERATOR_DFAGENERATOR_H_
#define GENERATOR_DFAGENERATOR_DFAGENERATOR_H_

namespace frontend::generator::dfa_generator {
using frontend::common::ObjectManager;
using frontend::generator::dfa_generator::nfa_generator::NfaGenerator;
namespace common = frontend::common;

class DfaGenerator {
  struct IntermediateDfaNode;

 public:
  //β�ڵ�����
  using TailNodeData = NfaGenerator::TailNodeData;
  //β�ڵ�ID
  using TailNodeId = TailNodeData::first_type;
  //β�ڵ����ȼ�
  using TailNodePriority = TailNodeData::second_type;
  // Nfa�ڵ�ID
  using NfaNodeId = NfaGenerator::NfaNodeId;
  //�Ӽ����취ʹ�õļ��ϵ�����
  using SetType = std::unordered_set<NfaNodeId>;
  //�м�ڵ�ID
  using IntermediateNodeId = ObjectManager<IntermediateDfaNode>::ObjectId;

  struct IntergalSetHasher {
    common::IntergalSetHashType DoHash(
        const std::unordered_set<NfaNodeId>& set) {
      return common::HashIntergalUnorderedSet(set);
    }
  };

  using SetManagerType =
      common::UnorderedStructManager<SetType, IntergalSetHasher>;
  //�Ӽ����취�õ����Ӽ���ID
  using SetId = SetManagerType::ObjectId;
  //�ַ���ǩ
  enum class WrapperLabel { kTransformArrayId };
  //״̬ת�Ʊ�ID
  using TransformArrayId =
      frontend::common::ExplicitIdWrapper<size_t, WrapperLabel,
                                          WrapperLabel::kTransformArrayId>;
  // DFA״̬ת�Ʊ�
  using TransformArray = std::array<TransformArrayId, common::kCharNum>;
  // DFA��������
  using DfaConfigType = std::vector<std::pair<TransformArray, TailNodeId>>;

  DfaGenerator() : head_node_intermediate_(IntermediateNodeId::InvalidId()) {}
  DfaGenerator(const DfaGenerator&) = delete;
  DfaGenerator(DfaGenerator&&) = delete;

  //��ӹؼ���
  bool AddKeyword(const std::string& str, TailNodeId tail_node_tag,
                  TailNodePriority priority_tag);
  //�������
  bool AddRegexpression(const std::string& str, TailNodeId tail_node_tag,
                        TailNodePriority priority_tag);
  //����DFA
  bool DfaConstruct();
  //������С��DFA
  bool DfaMinimize();
  //���½���������������
  bool DfaReconstrcut() { return DfaConstruct() && DfaMinimize(); }
  //������д��Config/DFA Config�ļ�����DFA.conf
  bool ConfigConstruct();

 private:
  struct IntermediateDfaNode {
    IntermediateDfaNode(SetId handler = SetId::InvalidId(),
                        TailNodeData data = NfaGenerator::NotTailNodeTag)
        : tail_node_data(data), set_handler(handler) {
      SetAllUntransable();
    }

    //������������������ת��
    void SetAllUntransable() {
      forward_nodes.fill(IntermediateNodeId::InvalidId());
    }
    SetId GetSetHandler() { return set_handler; }
    void SetTailNodeData(TailNodeData data) { tail_node_data = data; }

    //�ýڵ��ת���������洢IntermediateDfaNode�ڵ���
    std::array<IntermediateNodeId, common::kCharNum> forward_nodes;
    TailNodeData tail_node_data;
    SetId set_handler;  //�ýڵ��Ӧ���Ӽ��հ�
  };
  IntermediateDfaNode& GetIntermediateNode(IntermediateNodeId handler) {
    return node_manager_intermediate_node_.GetObject(handler);
  }
  SetType& GetSetObject(SetId id) { return node_manager_set_.GetObject(id); }
  //����ת�ƺ������Ӽ����취�ã���
  //����ֵǰ�벿�ֱ�ʾ�¼����Ƿ��Ѵ��ڣ���벿��Ϊ��Ӧ�м�ڵ���
  std::pair<bool, IntermediateNodeId> SetGoto(SetId set_src, char c_transform);
  // DFAת�ƺ�������С��DFA�ã�
  IntermediateNodeId IntermediateGoto(IntermediateNodeId dfa_src,
                                      char c_transform);
  //����DFA�м�ڵ�����ת��
  bool SetIntermediateNodeTransform(IntermediateNodeId node_intermediate_src,
                                    char c_transform,
                                    IntermediateNodeId node_intermediate_dst);
  //��������Ѵ����򷵻�true���������������벢����false
  //���صĵ�һ������Ϊ�����Ƿ��Ѵ���
  //���صĵڶ�������Ϊ��Ӧ�м�ڵ���
  std::pair<bool, IntermediateNodeId> InOrInsert(
      const SetType& uset,
      TailNodeData tail_node_data = NfaGenerator::NotTailNodeTag);

  //��handlers�����еľ������С��������c_transformת��������ʼ
  bool DfaMinimize(const std::vector<IntermediateNodeId>& handlers,
                   char c_transform);
  template <class IdType>
  bool DfaMinimizeGroupsRecursion(
      std::map<IdType, std::vector<IntermediateNodeId>> groups,
      char c_transform);

  //���л�����DFA������
  template <class Archive>
  void Serialize(Archive& ar, const unsigned int version = 0);

  // DFA����
  DfaConfigType dfa_config;
  //ͷ������
  size_t head_index;

  NfaGenerator nfa_generator_;
  //�м�ڵ�ͷ�����
  IntermediateNodeId head_node_intermediate_;
  //������Ч�ڵ���
  size_t config_node_num;

  //�洢DFA�м�ڵ㵽���ձ�ŵ�ӳ��
  std::unordered_map<IntermediateNodeId, TransformArrayId>
      intermediate_node_to_final_node_;
  //�洢�м�ڵ�
  ObjectManager<IntermediateDfaNode> node_manager_intermediate_node_;
  //�洢���ϵĹ�ϣ���ڵ�����ӳ��
  SetManagerType node_manager_set_;
  std::unordered_map<SetId, IntermediateNodeId> setid_to_intermediate_nodeid_;
};

template <class IdType>
inline bool DfaGenerator::DfaMinimizeGroupsRecursion(
    std::map<IdType, std::vector<IntermediateNodeId>> groups,
    char c_transform) {
  for (auto& p : groups) {
    std::vector<IntermediateNodeId>& vec = p.second;
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
