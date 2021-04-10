#pragma once

#include <array>

#include "NfaGenerator/NfaGenerator.h"
#include "common.h"
#include<map>

class DfaGenerator {
  struct IntermediateDfaNode;

 public:
  using TailNodeTag = NfaGenerator::TailNodeData::first_type;
  using PriorityTag = NfaGenerator::TailNodeData::second_type;
  using TailNodeData = NfaGenerator::TailNodeData;
  using NfaNodeHandler = NfaGenerator::NodeHandler;
  using SetType = std::unordered_set<NfaNodeHandler>;
  using IntermediateNodeHandler = NodeManager<IntermediateDfaNode>::NodeHandler;
  using SetNodeHandler = NodeManager<SetType>::NodeHandler;

  using TransformArray = std::array<size_t,kchar_num>;

  //DFA��������
  using DfaConfigType =
      std::vector<std::pair<TransformArray, TailNodeTag>>;

  DfaGenerator() :head_node_intermediate_(-1) {}
  DfaGenerator(const DfaGenerator&) = delete;
  DfaGenerator(DfaGenerator&&) = delete;

  //��ӹؼ���
  bool AddKeyword(const std::string& str, TailNodeTag tail_node_tag,
                  PriorityTag priority_tag);
  //�������
  bool AddRegexpression(const std::string& str, TailNodeTag tail_node_tag,
                        PriorityTag priority_tag);
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
    IntermediateDfaNode(SetNodeHandler handler = -1,
                        TailNodeData data = NfaGenerator::NotTailNodeTag)
        : tail_node_data(data), set_handler(handler) {
      SetAllUntransable();
    }

    //������������������ת��
    void SetAllUntransable() { forward_nodes.fill(-1); }
    SetNodeHandler GetSetHandler() { return set_handler; }
    void SetTailNodeData(TailNodeData data) { tail_node_data = data; }

    //�ýڵ��ת���������洢IntermediateDfaNode�ڵ���
    std::array<IntermediateNodeHandler, kchar_num> forward_nodes;
    TailNodeData tail_node_data;
    SetNodeHandler set_handler;  //�ýڵ��Ӧ���Ӽ��հ�
  };
  IntermediateDfaNode* GetIntermediateNode(IntermediateNodeHandler handler) {
    return node_manager_intermediate_node_.GetNode(handler);
  }
  SetType* GetSetNode(SetNodeHandler handler) {
    return node_manager_set_.GetNode(handler);
  }
  //����ת�ƺ������Ӽ����취�ã���
  //����ֵǰ�벿�ֱ�ʾ�¼����Ƿ��Ѵ��ڣ���벿��Ϊ��Ӧ�м�ڵ���
  std::pair<bool, IntermediateNodeHandler> SetGoto(SetNodeHandler set_src,
                                                   char c_transform);
  // DFAת�ƺ�������С��DFA�ã�
  IntermediateNodeHandler IntermediateGoto(IntermediateNodeHandler dfa_src, char c_transform);
  //����DFA�м�ڵ�����ת��
  bool SetIntermediateNodeTransform(
      IntermediateNodeHandler node_intermediate_src, char c_transform,
      IntermediateNodeHandler node_intermediate_dst);
  //��������Ѵ����򷵻�true���������������벢����false
  //���صĵ�һ������Ϊ�����Ƿ��Ѵ���
  //���صĵڶ�������Ϊ��Ӧ�м�ڵ���
  std::pair<bool, IntermediateNodeHandler> InOrInsert(
      const SetType& uset,
      TailNodeData tail_node_data = NfaGenerator::NotTailNodeTag);

  //��handlers�����еľ������С��������c_transformת��������ʼ
  bool DfaMinimize(const std::vector<IntermediateNodeHandler>& handlers,
                   char c_transform);
  template<class T>
  bool DfaMinimizeGroupsRecursion(
      std::map<T, std::vector<IntermediateNodeHandler>> groups);

  //���л�����DFA������
  template <class Archive>
  void Serialize(Archive& ar, const unsigned int version = 0);

  //DFA����
  DfaConfigType dfa_config;
  //ͷ������
  size_t head_index;

  NfaGenerator nfa_generator_;
  //�м�ڵ�ͷ�����
  IntermediateNodeHandler head_node_intermediate_;
  //������Ч�ڵ���
  size_t config_node_num;

  //�洢DFA�м�ڵ㵽���ձ�ŵ�ӳ��
  std::unordered_map<IntermediateNodeHandler, size_t>
      intermediate_node_to_final_node_;
  //�洢�м�ڵ�
  NodeManager<IntermediateDfaNode> node_manager_intermediate_node_;
  //�洢���ϣ��Ӽ����취�õ��ģ�
  NodeManager<SetType> node_manager_set_;
  //�洢���ϵĹ�ϣ���ڵ�����ӳ��
  std::unordered_multimap<IntergalSetHashType, IntermediateNodeHandler>
      set_hash_to_intermediate_node_handler_;
};

template <class T>
inline bool DfaGenerator::DfaMinimizeGroupsRecursion(
    std::map<T, std::vector<IntermediateNodeHandler>> groups) {
  for (auto& p : groups) {
    std::vector<IntermediateNodeHandler>& vec = p.second;
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
        node_manager_intermediate_node_.RemoveNode(vec[i]);
      }
      ++config_node_num;
    }
  }
  return false;
}
